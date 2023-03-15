#include "./../includes/Webserv.hpp"

int main() {
    Webserv webserv;
    Server server;

    // server._conf._return.first = true;
    // server._conf._return.second.code = 302;
    // server._conf._return.second.location = "example.com";
    // server._conf._autoindex = true;
    // server._conf._client_max_boby_size = 50;
    webserv.addServer(server);
    if (webserv.init())
        webserv.startMonitoring();
}