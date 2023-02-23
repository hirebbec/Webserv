#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>
#include <iostream>

int main() {
    int sockfd = socket(AF_INET, SOCK_STREAM, 0); // создаем TCP-сокет
    if (sockfd == -1) {
        std::cerr << "Failed to create socket.\n";
        return 1;
    }

    sockaddr_in server_addr;
    std::memset(&server_addr, 0, sizeof(server_addr)); // обнуляем структуру
    server_addr.sin_family = AF_INET; // семейство адресов IPv4
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1"); // адрес сервера (локальный)
    server_addr.sin_port = htons(8080); // порт сервера

    if (connect(sockfd, (sockaddr*)&server_addr, sizeof(server_addr)) != 0) { // подключаемся к серверу
        std::cerr << "Failed to connect to server.\n";
        return 1;
    }

    std::string message = "Hello, server!"; // сообщение для отправки
    int bytes_sent = send(sockfd, message.c_str(), message.length(), 0); // отправляем сообщение
    if (bytes_sent == -1) {
        std::cerr << "Failed to send message.\n";
        return 1;
    }

    std::cout << "Sent " << bytes_sent << " bytes: " << message << "\n";

    close(sockfd); // закрываем соединение

    return 0;
}
