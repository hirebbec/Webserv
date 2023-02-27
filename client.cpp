#include <sys/socket.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <iostream>
#include <string.h>
#include <unistd.h>

#define PORT    5555
#define BUFLEN  512

int writeToServer(int sock);
int readFromServer(int sock);

int main() {
    int sock, new_sock;
    struct sockaddr_in addr;
    struct sockaddr_in client;
    fd_set active_set, read_set;
    socklen_t size;
    char buf[BUFLEN];

    addr.sin_family = AF_INET;
    addr.sin_port = htons(PORT);

    sock = socket(PF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        std::cerr << "On client: socket creation failer\n";
        return -1;
    }

    if (connect(sock, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        std::cerr << "On client: connection failer\n";
        return -1;
    }

    std::cout << "Connection established\n";

    while (1) {
        if (writeToServer(sock) < 0)
            break;
        if (readFromServer(sock) < 0)
            break;
    }

    close(sock);
}

int readFromServer(int sock) {
    int nbytes;
    char* message[BUFLEN];

    nbytes = recv(sock, message, BUFLEN, 0);
    if (nbytes < 0) {
        std::cerr << "On client: read failer\n";
        return -1;
    } else if (nbytes == 0) {
        std::cout << "No message from server\n";
    } else {
        std::cout << "Message from server: " << message << std::endl;
    }
    return 0;
}

int writeToServer(int sock) {
    int nbytes;
    std::string message;

    std::cout << "Message to server: ";
    std::cin >> message;

    nbytes = send(sock, message.c_str(), message.length(), 0);
    if (nbytes < 0) {
        std::cerr << "On client: write failer\n";
        return -1;
    }
    if (nbytes == 0 || message == "stop") {
        return -1;
    }
    return 0;
}