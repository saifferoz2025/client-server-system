#include <iostream>
#include <unistd.h>
#include <arpa/inet.h>
#include <string.h>
using namespace std;

int main() {
    int sock = 0;
    struct sockaddr_in serv_addr;
    char buffer[1024] = {0};

    sock = socket(AF_INET, SOCK_STREAM, 0);
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(8080);
    serv_addr.sin_addr.s_addr = inet_addr("127.0.0.1");

    connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr));

    cout << "---- Welcome to MSF Ecommerce Store ----\n";
    cout << "Products Available:\n";
    cout << "101: Shirt - Rs.800\n102: Jeans - Rs.1200\n103: Shoes - Rs.2500\n";

    string id, qty, method, money;

    cout << "Enter Product ID: ";
    cin >> id;
    send(sock, id.c_str(), id.length(), 0);
    sleep(1);

    cout << "Enter Quantity: ";
    cin >> qty;
    send(sock, qty.c_str(), qty.length(), 0);
    sleep(1);

    // Receive total amount
    read(sock, buffer, 1024);
    cout << "[Server] " << buffer << endl;

    cout << "Only 'card' payment method is allowed.\n";
    cout << "Enter Payment Method: ";
    cin >> method;
    send(sock, method.c_str(), method.length(), 0);
    sleep(1);

    cout << "Enter Your Available Money: ";
    cin >> money;
    send(sock, money.c_str(), money.length(), 0);

    // Receive final result
    memset(buffer, 0, sizeof(buffer));
    read(sock, buffer, 1024);
    cout << " Server " << buffer << endl;

    return 0;
}
