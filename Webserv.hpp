#include "Server.hpp"
#include "HttpParser.hpp"
#include "HttpResponse.hpp"
#include <sys/select.h>
#include <arpa/inet.h>
#include <cstdio>
#include <fstream>

#define BUFFER_SIZE 4096

class Webserv {
private:
    std::vector<Server>		_servers;
    std::set<int>			_listen_socks;
    fd_set					_active_set;
    fd_set					_read_set;
    int						_max_sock;
	HttpParser				httpParser;
	HttpResponse			httpResponse;

public:
	Webserv(std::vector<Server> &servers): _servers(servers), _max_sock(0) {
		FD_ZERO(&_active_set);
	};

	Webserv(): _max_sock(0) {
		FD_ZERO(&_active_set);
	};

	void addServer(Server &server) {
		_servers.push_back(server);
	}

	bool init() {
		for (std::vector<Server>::iterator it = _servers.begin(); it != _servers.end(); ++it) {
			if ((*it).init()) {
				_max_sock = _max_sock > (*it).getSock() ? _max_sock : (*it).getSock();
				_listen_socks.insert((*it).getSock());
        		FD_SET((*it).getSock(), &_active_set);
				return true;
			}
		}
		return false;
	}

	bool startMonitoring() {
		while (true) {
			_read_set = _active_set;
			if (select(_max_sock + 1, &_read_set, NULL, NULL, NULL) < 0) {
				std::cerr << "On server: select failer\n";
				return false;
			}
			for (int sock = 0; sock <= _max_sock; ++sock) {
				if (FD_ISSET(sock, &_read_set)) {
					if (_listen_socks.count(sock) == 1) { // запрос на новое соединение
						acceptConnection(sock);
					} else {
						if (readRequest(sock))
							sendAnswer(sock);
					}
				}
			}
		}
		return true;
	}

private:
	bool acceptConnection(int sock) {
		int new_sock = accept(sock, NULL, NULL);
		if (new_sock < 0) {
			std::cerr << "On server: accept failer\n";
			return false;
		}
		_max_sock = _max_sock > new_sock ? _max_sock : new_sock;
		std::cout << "On server: new connection\n";
		FD_SET(new_sock, &_active_set);
		return true;
	}

	bool readRequest(int sock) {
		char buf[BUFFER_SIZE];
		int bytes = recv(sock, buf, BUFFER_SIZE, 0);
		if (bytes < 0) {
			std::cerr << "Recv error\n";
			return false;
		} else if (bytes == 0) {
			//TODO status 204(No content)
			return false;
		}
		if (!httpParser.parse(buf, bytes)) {
			//TODO status 400(Bad request)
			return false;	
		}
		return true;
	}

	void sendAnswer(int sock) {
		std::string response = httpResponse.generateResponse("./error_pages/404");
				send(sock, response.c_str(), response.length(), 0);

		Server server = choseServer(sock);
		Configuration conf = choseConf(server);
		if (conf._limit_except.methods.find(httpParser.method) == conf._limit_except.methods.end() ||
		conf._limit_except.methods[httpParser.method] == false) { // case 2.1
			// TODO status 405(Method Not Allowed) + Allow header
		} else if (conf._return.first) { //case 2.2
			// TODO status 302 + Location header
		} else if (httpParser.method == "GET") { // case 2.3
			if (httpParser.uri[httpParser.uri.length() - 1] == '/') { // case 2.3.1
				if (conf._index.first) { // case 2.3.1.1
					if (exist(conf._root + conf._index.second)) {
						// TODO return file conten
						// TODO successful response
					} else {
						// status 404
					}
				} else { // case 2.3.1.2
					if (conf._autoindex) {
						// TODO generate special page
						// TODO successful response
					} else {
						// status 404
					}
				}
			} else if (httpParser.uri.length() > 9 && httpParser.uri.substr(0, 9) == "/cgi-bin/") { // case 2.3.2
				// TODO cgi-script
			} else {
				if (exist(conf._root + httpParser.uri)) {
					// TODO return file conten
					// TODO successful response
				} else {
					// status 404
				}
			}
		} else if (httpParser.method == "POST") { // case 2.4
			if (httpParser.uri[httpParser.uri.length() - 1] == '/') { // case 2.4.1
				// TODO status 405
			} else if (httpParser.body.length() > conf._client_max_boby_size) {
				// TODO status 413
			} else {
				// TODO save file in special directory
				// TODO successful response
			}
		} else { // case 2.5
			if (httpParser.uri[httpParser.uri.length() - 1] == '/') { // case 2.5.1
				// TODO status 405
			} else { // case 2.5.2
				std::string path = conf._root + httpParser.uri;
				if (std::remove((const char*)path.c_str()) != 0) {
					// status 404
				} else {
					// TODO successful response
				}
			}
		}
		send(sock, response.c_str(), response.length(), 0); // send response
	}

	Server &choseServer(int sock) {
		for (std::vector<Server>::iterator it = _servers.begin(); it != _servers.end(); ++it) {
			if ((*it).getSock() == sock) {
				return *it;
			}
			return *_servers.end();
		}
	}

	Configuration &choseConf(Server &server) {
		Configuration conf = server._conf;
		std::string uri = httpParser.uri;
		while (uri.length() > 0) {
			if (server._locations.find(uri) != server._locations.end()) {
				return server._locations[uri]._conf;
			}
			for (int i = uri.length() - 1; i > 0; ++i) {
				if (uri[i] == '/') {
					uri.erase(0, i);
					break;
				}
			}
		}
		return conf;
	}

	bool exist(std::string path) {
		std::ifstream file(path);
		return file.good();
	}

};