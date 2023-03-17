#include "./../includes/Webserv.hpp"

int main() {
    Webserv webserv;
    Server server;

    server._server_name.push_back("localhost");
	server._port = "4040";
    server._conf._root = "/var/localhost/files";
    server._conf._limit_except.methods["POST"] = false;
    server._conf._limit_except.methods["DELETE"] = false;


    Location loc1("/uploads/");
    loc1._conf._client_max_boby_size = 4096;
    loc1._conf._root = "/var/localhost";
    loc1._conf._autoindex = true;

    Location loc2("/cgi-bin/");
    loc2._conf._limit_except.methods["POST"] = false;
    loc2._conf._limit_except.methods["DELETE"] = false;
    loc2._conf._error_pages.insert(std::make_pair(404, "/error_pages/4040.html"));
    loc2._conf._autoindex = true;

    Location loc3("/developer_files/");
    loc3._conf._return.first = true;
    loc3._conf._return.second.code = 403;
    loc3._conf._return.second.location = "/error_pages/403.html";

    Location loc4("/");
    loc4._conf._limit_except.methods["POST"] = false;
    loc4._conf._limit_except.methods["DELETE"] = false;
    loc4._conf._root = "/var/localhost/files";
    loc4._conf._index = "lol.html";

    Location loc5("/error_pages/");
    loc5._conf._autoindex = true;
    loc5._conf._limit_except.methods["POST"] = false;
    loc5._conf._limit_except.methods["DELETE"] = false;

    server._locations.insert(std::make_pair(loc1._uri, loc1));
    server._locations.insert(std::make_pair(loc2._uri, loc2));
    server._locations.insert(std::make_pair(loc3._uri, loc3));
    server._locations.insert(std::make_pair(loc4._uri, loc4));
    server._locations.insert(std::make_pair(loc5._uri, loc5));

    webserv.addServer(server);
    if (webserv.init())
        webserv.startMonitoring();
}