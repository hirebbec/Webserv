#include <sys/socket.h>
#include <iostream>
#include <netinet/in.h>
#include <unistd.h>
#include <cerrno>

#define PORT 8080

int main() {
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1) {
        std::cerr << "Failed to create socket. errno: " << errno << std::endl;
        return errno;
    }
    
    // Listen to port 8080 on any address
    sockaddr_in sockaddr;
    sockaddr.sin_family = AF_INET;
    sockaddr.sin_addr.s_addr = INADDR_ANY;
    sockaddr.sin_port = htons(PORT); // htons is necessary to convert a number to
                                 // network byte order
    
    if (!bind(sockfd, (struct sockaddr*)&sockaddr, sizeof(sockaddr)) < 0) {
        std::cout << "Failed to bind socket to port " << PORT << ". errno: " << errno << std::endl;
        return errno;
    }
  
    // Start listening. Hold at most 5 connections in the queue
    if (listen(sockfd, 5) < 0) {
        std::cout << "Failed to listen on socket. errno: " << errno << std::endl;
        return errno;
    }

    // Grab a connection from the queue
    int addrlen  = 0;
    int connection = accept(sockfd, (struct sockaddr*)&sockaddr, (socklen_t*)&addrlen);
    if (connection < 0) {
        std::cout << "Failed to grab connection. errno: " << errno << std::endl;
        return(errno);
    }

    char buffer[100];
    int bytesRead = read(connection, buffer, 100);
    std::cout << "The message was: " << buffer;

    send(connection, "Hello from server!\n", 20, 0);

    close(connection);
    close(sockfd);

}