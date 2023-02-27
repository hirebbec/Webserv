#include <sys/socket.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <iostream>
#include <string.h>
#include <unistd.h>

#define PORT    5555
#define BUFLEN  512

int readFromClient(int sock, char* buf);
void writeToClient(int sock, char *buf);

int main() {
    int sock, new_sock;
    struct sockaddr_in addr;
    struct sockaddr_in client;
    fd_set active_set, read_set;
    socklen_t size;
    char buf[BUFLEN];

    addr.sin_family = AF_INET;
    addr.sin_port = htons(PORT);
    addr.sin_addr.s_addr = htonl(INADDR_ANY);

    sock = socket(PF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        std::cerr << "On server: socket creation failer\n";
        return -1;
    }
    // setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, ) // TODO:

    if (bind(sock, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        std::cerr << "On server: cannot bind socket\n";
        return -1;
    }

    if (listen(sock, 5) < 0) {
        std::cerr << "On server: listen failer\n";
        return -1;
    }

    FD_ZERO(&active_set);
    FD_SET(sock, &active_set);

    while (1) {
        read_set = active_set;
        if (select(FD_SETSIZE, &read_set, NULL, NULL, NULL) < 0) {
            std::cerr << "On server: select failer\n";
            return -1;
        }

        for (int i = 0; i < FD_SETSIZE; ++i) {
            if (FD_ISSET(sock, &read_set)) {
                if (i == sock) { // запрос на новое соединение
                    size = sizeof(client);
                    new_sock = accept(sock, (struct sockaddr *)&client, &size);
                    if (new_sock < 0) {
                        std::cerr << "On server: accept failer\n";
                    }
                    FD_SET(new_sock, &active_set);
                } else {
                    int err = readFromClient(i, buf);
                    if (err < 0) {
                        close(i);
                        FD_CLR(i, &active_set);
                    } else {
                        writeToClient(i, buf);
                    }
                }
            }
        }
    }
}

int readFromClient(int sock, char* buf) {
    int nbytes;

    nbytes = recv(sock, buf, BUFLEN, 0);
    if (nbytes < 0) {
        std::cerr << "On server: read failer\n";
        return -1;
    } else if (nbytes == 0) {
        return -1;
    } else {
        if (strstr(buf, "stop")) {
            return -1;
        }
        std::cout << "Server got message: " << buf << std::endl;
        return 0;
    }
}

void writeToClient(int sock, char *buf) {
    int nbytes;

    for (int i = 0; i < strlen(buf) / 2; ++i) {
        char tmp = buf[i];
        buf[i] = buf[strlen(buf) - 1];
        buf[strlen(buf) - 1] = tmp;
    }
    nbytes = send(sock, buf, strlen(buf), 0);
    if (nbytes < 0) {
        std::cerr << "On server: write failer\n";
    }
}