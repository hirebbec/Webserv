#include <string>
#include <sstream>
#include <fstream>

class HttpResponse {
public:
    HttpResponse() {}

    std::string generateResponse(int statusCode, std::vector<std::string> headers, std::string path) {
        std::ostringstream oss;
        oss << "HTTP/1.1 " << statusCode << " " << getStatusMessage(statusCode) << "\r\n";
        for (std::vector<std::string>::iterator it = headers.begin(); it != headers.end(); ++it) {
            oss << (*it) << "\r\n";
        }
        std::ifstream file(path.c_str());
        if (file.is_open()) {
            std::string content((std::istreambuf_iterator<char>(file)),
                                (std::istreambuf_iterator<char>()));
            if (content.length() > 0) {
                oss << "Content-Type: " << "text/html" << "\r\n";
                oss << "Content-Length: " << content.length() << "\r\n\r\n";
                oss << content << "\r\n";
            }
            file.close();
        }
        return oss.str();
    }

private:
    std::string getStatusMessage(int statusCode) {
        switch (statusCode) {
            case 200:
                return "OK";
            case 204:
                return "No Content";
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
