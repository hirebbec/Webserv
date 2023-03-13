#include "Location.hpp"
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <cstdlib>
#include <iostream>


struct Server: Location {
public:	
	Configuration					_conf;
	std::string						_port;
	std::vector<std::string>		_server_name;
	std::map<std::string, Location>	_locations;

private:
	sockaddr_in _addr;
	int			_sock;

	bool init_sockaddr() {
		struct addrinfo hints, *res;
		hints.ai_flags = 0;                // никаких флагов
		hints.ai_family = AF_UNSPEC;       // IPv4 или IPv6
		hints.ai_socktype = SOCK_STREAM;   // потоковый сокет
		hints.ai_protocol = IPPROTO_TCP;   // TCP-протокол
		hints.ai_addrlen = 0;
		hints.ai_canonname = NULL;
		hints.ai_addr = NULL;
		hints.ai_next = NULL;
		if (getaddrinfo(_server_name[0].c_str(), _port.c_str(), &hints, &res) != 0) {
			std::cerr << "Getaddrinfo error on server " << _server_name[0] << std::endl;
			return false;
		}
		_addr.sin_family = AF_INET;
		_addr.sin_port = htons(std::atoi(_port.c_str()));
		_addr.sin_addr = ((struct sockaddr_in *) res->ai_addr)->sin_addr;
		freeaddrinfo(res);
		return true;
	}

	bool create_socket() {
		_sock = socket(AF_INET, SOCK_STREAM, 0);
		if (_sock < 0) {
			std::cerr << "Socket creating error on server: " << _server_name[0] << std::endl;
			return false;
		}
		return true;
	}

	bool binding() {
		if (bind(_sock, (struct sockaddr*)&_addr, sizeof(_addr)) < 0) {
			std::cerr << "Binding error on server: " << _server_name[0] << std::endl;
			return false;
		}
		return true;
	}

	bool listening() {
		if (listen(_sock, 16) < 0) {
        	std::cerr << "Listening error on server: " << _server_name[0] << std::endl;
        	return false;
    	}
		return true;
	}

public:
	bool init() {
		if (this->init_sockaddr() && this->create_socket() && \
			this->binding() && 	this->listening())
			return true;
		return false;
	}

	int getSock() {
		return _sock;
	}

	Server() {
		_server_name.push_back("localhost");
		_port = "132";
	};
};