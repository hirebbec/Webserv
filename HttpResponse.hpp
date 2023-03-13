#include <string>
#include <sstream>
#include <fstream>

class HttpResponse {
public:
    HttpResponse() {}

    std::string generateResponse(int statusCode = 200, std::string contentType = "text/plain", std::string content = "") {
        std::ostringstream oss;
        oss << "HTTP/1.1 " << statusCode << " " << getStatusMessage(statusCode) << "\r\n";
        oss << "Content-Type: " << contentType << "\r\n";
        oss << "Content-Length: " << content.length() << "\r\n";
        oss << "\r\n";
        oss << content;
        return oss.str();
    }

    std::string generateResponse(std::string path) {
        std::ifstream file(path);
        if (file.is_open()) {
            std::string content((std::istreambuf_iterator<char>(file)),
                                (std::istreambuf_iterator<char>()));
            file.close();
            return content;
        }
        return "HTTP/1.1 404 Not Found\n\rContent-Type: text/html\n\rContent-Length: 149\n\r\n\r<html>\n\r<head><title>404 Not Found</title></head>\n\r<body>\n\r<h1>404 Not Found</h1>\n\r<p>The requested URL was not found on this server.</p>\n\r</body>\n\r</html>";
    }

private:
    std::string getStatusMessage(int statusCode) {
        switch (statusCode) {
            case 200:
                return "OK";
            case 302:
                return "Found";
            case 400:
                return "Bad Request";
            case 404:
                return "Not Found";
            case 405:
                return "Method Not Allowed";
            case 413:
                return "Payload Too Large";
            default:
                return "Unknown";
        }
    }
};
