#include <string>
#include <map>

class HttpParser {
public:
    std::string method;  // метод запроса (GET, POST, PUT, DELETE и т.д.)
    std::string uri;     // URI ресурса, на который отправляется запрос
    std::string version; // версия протокола HTTP

    std::map<std::string, std::string> headers; // заголовки запроса
    std::string body;   // тело запроса

    bool parse(const char* data, size_t length) {
        std::string request(data, length);
        clear();

        size_t pos1 = request.find(' ');  // ищем первый пробел - разделитель между методом, URI и версией
        size_t pos2 = request.find(' ', pos1+1);
        if (pos1 == std::string::npos || pos2 == std::string::npos) {
            return false;
        }

        method = request.substr(0, pos1);
        uri = request.substr(pos1+1, pos2-pos1-1);
        version = request.substr(pos2+1, request.find("\r\n")-pos2-1);

        // парсим заголовки
        size_t pos = request.find("\r\n") + 2; // переходим на следующую строку
        while (pos < request.length() && request[pos] != '\r' && request[pos] != '\n') {
            size_t pos_colon = request.find(':', pos);
            if (pos_colon == std::string::npos) {
                return false;
            }

            std::string header_name = request.substr(pos, pos_colon - pos);
            pos_colon += 2; // пропускаем двоеточие и пробел
            size_t pos_end = request.find("\r\n", pos_colon);
            if (pos_end == std::string::npos) {
                return false;
            }

            std::string header_value = request.substr(pos_colon, pos_end - pos_colon);
            headers[header_name] = header_value;

            pos = pos_end + 2; // переходим на следующую строку
        }

        // парсим тело запроса
        if (headers.find("Content-Length") != headers.end()) {
            size_t content_length = atoi(headers["Content-Length"].c_str());
            if (request.length() < pos + content_length) {
                return false;
            }

            body = request.substr(pos + 2, content_length + 2);
        }

        return true;
    }

    void clear() {
        this->body = "";
        this->headers.clear();
        this->method = "";
        this->version = "";
        this->uri = "";
    }
};
