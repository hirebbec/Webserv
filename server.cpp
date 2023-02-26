#include <iostream>
#include <sstream>
#include <string>
#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <fcntl.h>
#include <fstream>

const int PORT = 8080;

void handle_client(int client_socket) {
    // Чтение запроса от клиента
    char buffer[1024];
    int request_size = read(client_socket, buffer, sizeof(buffer));
    // printf("\n%s", buffer);

    // Проверка ошибок чтения
    if (request_size < 0) {
        std::cerr << "Error reading from socket" << std::endl;
        return;
    }

    // Парсинг HTTP-запроса
    std::istringstream request(buffer);
    std::string method, path, version;
    request >> method >> path >> version;

    // Проверка правильности метода и версии HTTP-протокола
    if (method != "GET" || version != "HTTP/1.1") {
        std::cerr << "Invalid request" << std::endl;
        return;
    }

    std::ostringstream response;
    std::string response_str;
    // Отправка ответа клиенту
    if (path == "/") {
        response << "HTTP/1.1 200 OK\r\n";
        response << "Content-Type: text/html\r\n";
        response << "Connection: close\r\n";
        response << "\r\n";
        response << "<html><body><h1>Home Page</h1></body></html>";
    } else {
        path.erase(path.begin());
        std::ifstream file(path);
        if (file.is_open())
        {
            // Read file contents
            std::stringstream file_contents;
            file_contents << file.rdbuf();

            // Close file
            file.close();

            // Build response
            response << file_contents.rdbuf();
        } else {
            response << "HTTP/1.1 200 OK\r\n";
            response << "Content-Type: text/html\r\n";
            response << "Connection: close\r\n";
            response << "\r\n";
            response << "<html><body><h1>Error 404, page not found!</h1></body></html>";
        }
    }
    send(client_socket, response.str().c_str(), response.str().size(), 0);
    printf("Reply sent\n");
    // Закрытие сокета
    close(client_socket);
}

int main() {
    // Создание серверного сокета
    int server_socket = socket(AF_INET, SOCK_STREAM, 0);

    // Проверка ошибок создания сокета
    if (server_socket < 0) {
        std::cerr << "Error creating socket" << std::endl;
        return 1;
    }

    // Настройка параметров серверного адреса
    struct sockaddr_in server_address;
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = INADDR_ANY;
    server_address.sin_port = htons(PORT);

    // Привязка сокета к адресу
    int bind_result = bind(server_socket, (struct sockaddr*)&server_address, sizeof(server_address));
    if (bind_result < 0) {
        std::cerr << "Error binding socket: " << strerror(errno) << std::endl;
        return 1;
    }

    // Ожидание подключения клиента
    listen(server_socket, 5);

    // Обработка клиентских запросов
    while (true) {
        struct sockaddr_in client_address;
        socklen_t client_address_size = sizeof(client_address);

        int client_socket = accept(server_socket, (struct sockaddr*)&client_address, &client_address_size);
        if (client_socket < 0) {
            std::cerr << "Error accepting client connection" << std::endl;
            continue;
        }

        // Обработка запроса от клиента
        handle_client(client_socket);
    }

    // Закрытие серверного сокета
    close(server_socket);

    return 0;
}
