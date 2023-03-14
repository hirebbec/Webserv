#include "./../includes/Webserv.hpp"

int main() {
    Webserv webserv;
    Server server;

    webserv.addServer(server);
    if (webserv.init())
        webserv.startMonitoring();
}