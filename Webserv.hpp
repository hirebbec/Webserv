#include "Server.hpp"
#include <sys/select.h>
#include <arpa/inet.h>

class Webserv {
private:
    std::vector<Server>	_servers;
    std::set<int>		_listen_socks;
    fd_set				_active_set;
    fd_set				_read_set;
    int					_max_sock;
	//HttpParser
	//HttpResponse

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
						readRequest(sock);
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
		std::cout << "\nNew message from client\n";
		/* Читаем запрос и отправяем ответ TODO*/
		send(sock, "HTTP/1.1 200 OK\n\rConnection: keep-alive\n\rContent-Type: text/plain\n\rContent-Length: 13\n\r\n\rHello, World!", 112, 0);
		return true;
	}
};