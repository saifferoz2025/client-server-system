#include <iostream>
#include <fstream>
#include <sstream>
#include <unistd.h>
#include <netinet/in.h>
#include <pthread.h>
#include <cstdlib>
#include <ctime>
#include <string>
#include <cstring>
using namespace std;

struct OrderInfo {
    int productID;
    int quantity;
    int pricePerItem;
    int totalAmount;
    int moneyPaid;
    string paymentMethod;
    string result;
};

int getPriceByProductID(int id) {
    ifstream fin("inventory.txt");
    string line;
    while (getline(fin, line)) {
        stringstream ss(line);
        string pid, name, price, stock;
        getline(ss, pid, ',');
        getline(ss, name, ',');
        getline(ss, price, ',');
        getline(ss, stock, ',');
        if (stoi(pid) == id) {
            return stoi(price);
        }
    }
    return -1;
}
void updateInventory(int id, int qty);
// Define payment processing function
void* processPayment(void* arg) {
            OrderInfo* order = (OrderInfo*)arg;
            if (order->paymentMethod != "card") {
            order->result = "Payment Failed: Only card payments allowed.";
            } else if (order->moneyPaid < order->totalAmount) {
            order->result = "Payment Failed: Insufficient funds.";
                   } else {
            order->result = "Payment Successful!";
            updateInventory(order->productID, order->quantity);
                }
            pthread_exit(NULL);
             }

void updateInventory(int id, int qty) {
    ifstream fin("inventory.txt");
    ofstream fout("temp.txt");
    string line;
    while (getline(fin, line)) {
        stringstream ss(line);
        string pid, name, price, stock;
        getline(ss, pid, ','); getline(ss, name, ',');
        getline(ss, price, ','); getline(ss, stock, ',');

        if (stoi(pid) == id) {
            int newStock = stoi(stock) - qty;
            fout << pid << "," << name << "," << price << "," << newStock << "\n";
        } else {
            fout << line << "\n";
        }
    }
    fin.close();
    fout.close();
    rename("temp.txt", "inventory.txt");
}

void logOrder(const OrderInfo& order, string clientIP) {
    ofstream log("orders.txt", ios::app);
    time_t now = time(0);
    log << "Client: " << clientIP << "\n";
    log << "Product ID: " << order.productID
        << ", Quantity: " << order.quantity
        << ", Price Each: " << order.pricePerItem
        << ", Total: " << order.totalAmount << "\n";
    log << "Payment Method: " << order.paymentMethod
        << ", Paid: " << order.moneyPaid
        << ", Result: " << order.result << "\n";
    log << "Date: " << ctime(&now) << "------------------------\n";
    log.close();
}

int main() {
    int server_fd, new_socket;
    struct sockaddr_in address;
    int opt = 1, addrlen = sizeof(address);
    char buffer[1024] = {0};
    srand(time(0));

    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(8080);

    bind(server_fd, (struct sockaddr *)&address, sizeof(address));
    listen(server_fd, 5);

    cout << "[Server] Listening on port 8080...\n";

    while (true) {
        new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen);
        if (!fork()) {
            cout << "\n[Server] Client connected.\n";
            OrderInfo order;

            // Step 1: Receive Product ID
            memset(buffer, 0, sizeof(buffer));
            read(new_socket, buffer, 1024);
            order.productID = atoi(buffer);
            cout << "[Server] Product ID received: " << order.productID << endl;

            // Step 2: Receive Quantity
            memset(buffer, 0, sizeof(buffer));
            read(new_socket, buffer, 1024);
            order.quantity = atoi(buffer);
            cout << "[Server] Quantity received: " << order.quantity << endl;

            // Step 3: Calculate total
            order.pricePerItem = getPriceByProductID(order.productID);
            order.totalAmount = order.pricePerItem * order.quantity;

            string totalMsg = "Total Amount: " + to_string(order.totalAmount);
            send(new_socket, totalMsg.c_str(), totalMsg.length(), 0);
            cout << "[Server] Sent total amount: " << order.totalAmount << endl;

            // Step 4: Receive payment method
            memset(buffer, 0, sizeof(buffer));
            read(new_socket, buffer, 1024);
            order.paymentMethod = buffer;
            cout << "[Server] Payment method: " << order.paymentMethod << endl;

            // Step 5: Receive money
            memset(buffer, 0, sizeof(buffer));
            read(new_socket, buffer, 1024);
            order.moneyPaid = atoi(buffer);
            cout << "[Server] Money received: " << order.moneyPaid << endl;

            // Step 6:Threaded Payment validation
            pthread_t t;
            pthread_create(&t, NULL, processPayment, &order);
            pthread_join(t, NULL);

            // Step 7: Send result
            send(new_socket, order.result.c_str(), order.result.length(), 0);            cout << "[Server] Payment result sent: " << order.result << endl;

            // Step 8: Display summary on server
            cout << "\n======= ORDER SUMMARY (SERVER) =======" << endl;
            cout << "Client IP: 127.0.0.1" << endl;
            cout << "Product ID: " << order.productID << endl;
            cout << "Quantity: " << order.quantity << endl;
            cout << "Price Each: " << order.pricePerItem << endl;
            cout << "Total Amount: " << order.totalAmount << endl;
            cout << "Payment Method: " << order.paymentMethod << endl;
            cout << "Money Paid: " << order.moneyPaid << endl;
            cout << "Payment Result: " << order.result << endl;
            cout << "=======================================\n" << endl;

            // Step 9: Log order
            logOrder(order, "127.0.0.1");

            close(new_socket);
            exit(0);
        }
    }

    return 0;
}
