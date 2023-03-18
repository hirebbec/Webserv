#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <iostream>
#include <unistd.h>
#include <strings.h>
#include <fcntl.h>

int main() {
  const char *host = "localhost";
  int port = 4041;

  // Создаем сокет
  int sockfd = socket(AF_INET, SOCK_STREAM, 0);
  if (sockfd < 0) {
    std::cerr << "Error creating socket" << std::endl;
    return 1;
  }

  // Получаем IP-адрес сервера
  struct hostent *server = gethostbyname(host);
  if (server == NULL) {
    std::cerr << "Error resolving host" << std::endl;
    return 1;
  }

  // Создаем адрес сервера
  struct sockaddr_in server_addr;
  bzero((char *)&server_addr, sizeof(server_addr));
  server_addr.sin_family = AF_INET;
  bcopy((char *)server->h_addr,
        (char *)&server_addr.sin_addr.s_addr,
        server->h_length);
  server_addr.sin_port = htons(port);

  // Устанавливаем соединение
  if (connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
    std::cerr << "Error connecting to server" << std::endl;
    return 1;
  }

  //Отправляем запрос
  std::string request = "DELETE /uploads/lol HTTP/1.1\r\n";
  // std::string request = "1231312312";
  send(sockfd, request.c_str(), request.size(), 0);

  // Читаем ответ
  // char buffer[1024];
  // recv(sockfd, buffer, 1024, 0);

  // std::cout << (const char*)buffer;
  // Закрываем соединение

  // recv(sockfd, buffer, 1024, 0);
  // std::cout << (const char*)buffer;
  // request = "GET /cgi-bin/time HTTP/1.1\r\n";
  close(sockfd);
  return 0;
}

 
