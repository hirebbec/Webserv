#include "Server.hpp"
#include "HttpParser.hpp"
#include "HttpResponse.hpp"
#include <sys/select.h>
#include <arpa/inet.h>
#include <cstdio>
#include <fstream>
#include <ctime>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>


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
			std::string response = httpResponse.generateResponse(204, std::vector<std::string>(), "204");
			send(sock, response.c_str(), response.length(), 0); // No content
			return false;
		}
		if (!httpParser.parse(buf, bytes)) {
			std::string response = httpResponse.generateResponse(400, std::vector<std::string>(), "400");
			send(sock, response.c_str(), response.length(), 0); // Bad request
			return false;	
		}
		return true;
	}

	void sendAnswer(int sock) {
		int code;
		std::vector<std::string> headers;
		std::string path;

		Server server = choseServer(sock);
		Configuration conf = choseConf(server);
		if (conf._limit_except.methods.find(httpParser.method) == conf._limit_except.methods.end() ||
		conf._limit_except.methods[httpParser.method] == false) { // case 2.1
			code = 405; //Method Not Allowed
			headers.push_back(conf._limit_except.getAllowMethods()); // Allow header
		} else if (conf._return.first) { //case 2.2
			code = 302; // Found
			headers.push_back(conf._return.second.getReturnPath()); // Location header
		} else if (httpParser.method == "GET") { // case 2.3
			if (httpParser.uri[httpParser.uri.length() - 1] == '/') { // case 2.3.1
				if (conf._index.first) { // case 2.3.1.1
					if (exist(conf._root + conf._index.second)) {
						code = 200; // successful response
						path = conf._root + conf._index.second; // Index file path
					} else {
						code = 404; // Not Found
					}
				} else { // case 2.3.1.2
					if (conf._autoindex) {
						generate_autoindex(conf._root + httpParser.uri, sock); // autoindex
						return;
					} else {
						code = 404; // Not Found
					}
				}
			} else if (httpParser.uri.length() > 8 && httpParser.uri.substr(0, 8) == "cgi-bin/") { // case 2.3.2
				// TODO cgi-script
				if (exist(conf._root + httpParser.uri)){
					executeCGI(conf._root + httpParser.uri, sock);
					return;
				} else {
					code = 404; // Not Found
				}
			} else {
				if (exist(conf._root + httpParser.uri)) {
					code = 200; // successful response
					path = conf._root + httpParser.uri; // File path
				} else {
					code = 404; // Not Found
				}
			}
		} else if (httpParser.method == "POST") { // case 2.4
			if (httpParser.headers.find("Content-Type") == httpParser.headers.end() ||
			httpParser.headers.find("Content-Length") == httpParser.headers.end()) {
				code = 400; //Bad Request
			}
			else if (httpParser.uri[httpParser.uri.length() - 1] == '/') { // case 2.4.1
				code = 405; //Method Not Allowed
				headers.push_back(conf._limit_except.getAllowMethods()); // Allow header
			} else if (httpParser.body.length() > conf._client_max_boby_size) {
				code = 413; // Payload Too Large
			} else {
				saveContent(); // Saving
				code = 200; // successful response
			}
		} else { // case 2.5
			if (httpParser.uri[httpParser.uri.length() - 1] == '/') { // case 2.5.1
				code = 405; //Method Not Allowed
				headers.push_back(conf._limit_except.getAllowMethods()); // Allow header
			} else { // case 2.5.2
				std::string path = conf._root + httpParser.uri;
				if (std::remove((const char*)path.c_str()) != 0) {
					code = 404; // Not Found
				} else {
					code = 200; // successful response
				}
			}
		}
		path = getPath(code, conf);
		std::string response = httpResponse.generateResponse(code, headers, path);
		send(sock, response.c_str(), response.length(), 0);
	}

	Server &choseServer(int sock) {
		for (std::vector<Server>::iterator it = _servers.begin(); it != _servers.end(); ++it) {
			if ((*it).getSock() == sock) {
				return *it;
			}
		}
		return _servers[0];
	}

	Configuration choseConf(Server &server) {
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
		std::ifstream file(path.c_str());
		bool ans = file.good();
		file.close();
		return ans;
	}

	std::string	getPath(int code, const Configuration &conf) {
		if (conf._error_pages.find(code) != conf._error_pages.end()) {
			return (*conf._error_pages.find(code)).second;
		} else {
			std::stringstream ss;
			ss << code;
			std::string path("error_pages/");
			return path + ss.str();
		}
	}

	// Функция для получения времени создания файла в формате временной метки
	std::string get_file_create_time(const char* filename) {
		struct stat attrib;
		if (stat(filename, &attrib) != 0) {
			return "";
		}
		std::stringstream ss;
		ss << attrib.st_mtime;
		return ss.str();
	}

	// Функция для получения списка файлов и директорий в заданной директории
	std::vector<std::string> get_directory_contents(const std::string& directory) {
		std::vector<std::string> contents;
		DIR* dir = opendir(directory.c_str());
		if (dir == NULL) {
			return contents;
		}
		struct dirent* entry;
		while ((entry = readdir(dir)) != NULL) {
			std::string name = entry->d_name;
			if (name != "." && name != "..") {
				contents.push_back(name);
			}
		}
		closedir(dir);
		return contents;
	}

	// Функция для генерации страницы со списком файлов и директорий
	void generate_autoindex(const std::string& directory, int sock) {
		std::vector<std::string> contents = get_directory_contents(directory);
		std::stringstream ss;
		std::string response;
		ss << "<html><head><title>Index of " << directory << "</title></head><body>";
		ss << "<h1>Index of " << directory << "</h1>";
		ss << "<table>";
		ss << "<tr><th>Name</th><th>Last Modified</th></tr>";
		for (std::vector<std::string>::iterator it = contents.begin(); it != contents.end(); ++it) {
			std::string filename = directory + "/" + *it;
			std::string create_time = get_file_create_time(filename.c_str());
			ss << "<tr><td><a href=\"" << *it << "\">" << *it << "</a></td><td>" << create_time << "</td></tr>";
		}
		ss << "</table></body></html>";
		response.append("HTTP/1.1 200 OK\r\n");
		response.append("Content-Type: text/html\r\n");
		response.append("Content-Length: ");
		response.append(intToStr(ss.str().length()));
		response.append("\r\n");
		response.append("\r\n");
		response.append(ss.str());
		send(sock, response.c_str(), response.length(), 0);
	}

	void saveContent() {
		time_t now = time(NULL);
		struct tm *t = localtime(&now);
		char buf[80];
		std::strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", t);
		std::stringstream ss;
		ss << buf;
		std::string time_str = ss.str();
		std::ofstream fout;
		fout.open(("uploads/" + time_str).c_str());
		fout << httpParser.body;
		fout.close();
	}

	std::string intToStr(int num) {
		std::stringstream ss;
		ss << num;
		return ss.str();
	}

	void	executeCGI(std::string path, int sock) {
		int pid = fork();
		if (pid == 0) {
			dup2(sock, STDOUT_FILENO);
			execve(path.c_str(), NULL, NULL);
		}
		wait(NULL);
	}
};