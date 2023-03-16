#include "./../includes/Webserv.hpp"

int main() {
    Webserv webserv;
    Server server;

    Location loc1("/uploads/");
    // loc1._conf._limit_except.methods["GET"] = false;
    loc1._conf._limit_except.methods["DELETE"] = false;
    loc1._conf._client_max_boby_size = 100;
    loc1._conf._root = "var/localhost/uploads";
    loc1._conf._autoindex = true;

    Location loc2("/cgi-bin/");
    loc2._conf._limit_except.methods["POST"] = false;
    loc2._conf._limit_except.methods["DELETE"] = false;
    loc2._conf._error_pages.insert(std::make_pair(404, "error_pages/4040.html"));

    Location loc3("/includes/");
    loc3._conf._return.first = true;
    loc3._conf._return.second.code = 403;
    loc3._conf._return.second.location = "/error_pages/403.html";

    Location loc4("/srcs/");
    loc3._conf._return.first = true;
    loc3._conf._return.second.code = 403;
    loc3._conf._return.second.location = "/error_pages/403.html";

    Location loc5("/error_pages/");
    loc3._conf._return.first = true;
    loc3._conf._return.second.code = 403;
    loc3._conf._return.second.location = "/error_pages/403.html";

    Location loc6("/");
    loc6._conf._root = "var/localhost/files";
    loc6._conf._index = "index.html";
    loc6._conf._autoindex = true;

    server._locations.insert(std::make_pair(loc1._uri, loc1));
    server._locations.insert(std::make_pair(loc2._uri, loc2));
    server._locations.insert(std::make_pair(loc3._uri, loc3));
    server._locations.insert(std::make_pair(loc4._uri, loc4));
    server._locations.insert(std::make_pair(loc5._uri, loc5));
    server._locations.insert(std::make_pair(loc6._uri, loc6));

    webserv.addServer(server);
    if (webserv.init())
        webserv.startMonitoring();
}