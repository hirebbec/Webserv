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
#include <sys/types.h>
#include <sys/wait.h>


void parseURI(std::vector<std::string>& envp, std::string& uri);
char** vectorToCharArray(std::vector<std::string>& vec);

#define BUFFER_SIZE 165536

class Webserv {
private:
    std::vector<Server>						_servers;
    std::set<int>							_listen_socks;
    fd_set									_active_set;
    fd_set									_read_set;
    int										_max_sock;
	HttpParser								httpParser;
	HttpResponse							httpResponse;
	std::vector<std::string>				_env;
	std::vector<std::string>				_paths;
public:

	Webserv(std::vector<std::string> env, std::vector<std::string> paths): _max_sock(0), _env(env), _paths(paths) {
		FD_ZERO(&_active_set);
	};

	void addServer(Server &server) {
		_servers.push_back(server);
	}

	bool init() {
		int counter = 0;
		for (std::vector<Server>::iterator it = _servers.begin(); it != _servers.end(); ++it) {
			if ((*it).init()) {
				_max_sock = _max_sock > (*it).getSock() ? _max_sock : (*it).getSock();
				_listen_socks.insert((*it).getSock());
        		FD_SET((*it).getSock(), &_active_set);
				counter++;
			} else {
				continue;
			}
		}
		if (counter == 0)
			return false;
		return true;
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
						else
							delete_client(sock);
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
		for (std::vector<Server>::iterator it = _servers.begin(); it != _servers.end(); ++it) {
			if ((*it).getSock() == sock) {
				(*it)._clients.insert(new_sock);
				break;
			}
		}
		_max_sock = _max_sock > new_sock ? _max_sock : new_sock;
		FD_SET(new_sock, &_active_set);
		return true;
	}

	void delete_client(int sock) {
		for (std::vector<Server>::iterator it = _servers.begin(); it != _servers.end(); ++it) {
			(*it)._clients.erase(sock);
		}
	}

	bool readRequest(int sock) {
		char buf[BUFFER_SIZE];
		int i = 0;
		while (i < BUFFER_SIZE) {
			buf[i] = 0;
			i++;
		}
		int bytes = recv(sock, buf, BUFFER_SIZE, 0);

		if (bytes < 0) {
			std::cerr << "Error: Failed to receive data.\n";
			delete_client(sock);
			close(sock);
			FD_CLR(sock, &_active_set);
			return false;
		} 
		else if (bytes == 0) {
			std::string response = httpResponse.generateResponse(204, std::vector<std::string>(), "/error_page/204.html");
			if (send(sock, response.c_str(), response.length(), 0) <= 0) {
				delete_client(sock);
			}
			delete_client(sock);
			close(sock);
			FD_CLR(sock, &_active_set);
			return false;
		}

		if (!httpParser.parse(buf, bytes)) {
			std::string response = httpResponse.generateResponse(400, std::vector<std::string>(), "/error_page/400.html");
			if (send(sock, response.c_str(), response.length(), 0) <= 0) {
				delete_client(sock);
				FD_CLR(sock, &_active_set);
				close(sock);
			} 
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
		conf._limit_except.methods[httpParser.method] == false) { // case 2.1(Checked)
			code = 405; //Method Not Allowed
			headers.push_back(conf._limit_except.getAllowMethods()); // Allow header
		} else if (conf._return.first) { //case 2.2 (Checked)
			code = 302; // Found
			headers.push_back(conf._return.second.getReturnPath()); // Location header
		} else if (httpParser.method == "GET") { // case 2.3
			if (httpParser.uri[httpParser.uri.length() - 1] == '/') { // case 2.3.1
				if (exist(conf._root + httpParser.uri +  conf._index)) { // case 2.3.1.1 (Checked)
					std::string response = httpResponse.generateResponse(200, headers, conf._root + httpParser.uri +  conf._index);
					if (send(sock, response.c_str(), response.length(), 0) <= 0) {
						delete_client(sock);
						FD_CLR(sock, &_active_set);
						close(sock);
					}
					return;
				} else if (conf._autoindex) { // case 2.3.1.2 (Checked)
					generate_autoindex("." + conf._root + httpParser.uri, sock); // autoindex
					return;
				} else { // case 2.3.1.3 (Checked)
					code = 404; // Not Found
				}
			} else if (conf._root == "/cgi-bin") { // case 2.3.2 (Checked)
				std::string uri = conf._root + httpParser.uri;
				parseURI(_env, uri);
				if (exist(uri)) {
					executeScript(uri, sock);
					return;
				} else {
					code = 404; // Not Found
				}
			} else {
				if (exist(conf._root + httpParser.uri) && !is_directory(conf._root + httpParser.uri)) { // case 2.3.3 (Checked)
					code = 200; // successful response
				} else if (is_directory(conf._root + httpParser.uri) && exist(conf._root + httpParser.uri + conf._index)) {
					code = 200; // successful response
				}else {
					code = 404; // Not Found
				}
			}
		} else if (httpParser.method == "POST") { // case 2.4
			if (httpParser.headers.find("Content-Type") == httpParser.headers.end() ||
			httpParser.headers.find("Content-Length") == httpParser.headers.end()) {
				code = 400; //Bad Request
			}
			else if (httpParser.uri[httpParser.uri.length() - 1] == '/') { // case 2.4.1 (Checked)
				code = 405; //Method Not Allowed
				headers.push_back(conf._limit_except.getAllowMethods()); // Allow header
			} else if (httpParser.body.length() > conf._client_max_boby_size) { // case 2.4.2 (Checked)
				code = 413; // Payload Too Large
			} else { // case 2.4.3 (Checked)
				headers.push_back(saveContent(conf._root)); // Saving
				code = 201; // successful response
			}
		} else { // case 2.5
			if (httpParser.uri[httpParser.uri.length() - 1] == '/') { // case 2.5.1 (Checked)
				code = 405; //Method Not Allowed
				headers.push_back(conf._limit_except.getAllowMethods()); // Allow header
			} else { // case 2.5.2 (Checked)
				std::string path = "." + conf._root + httpParser.uri;
				if (std::remove((const char*)path.c_str()) != 0) {
					code = 404; // Not Found
				} else {
					code = 200; // successful response
				}
			}
		}
		path = getPath(code, conf);
		std::string response = httpResponse.generateResponse(code, headers, path);
		if (send(sock, response.c_str(), response.length(), 0) <= 0) {
			delete_client(sock);
			FD_CLR(sock, &_active_set);
			close(sock);
		}
	}

	Server &choseServer(int sock) {
		for (std::vector<Server>::iterator it = _servers.begin(); it != _servers.end(); ++it) {
			if ((*it)._clients.find(sock) != (*it)._clients.end()) {
				return *it;
			}
		}
		return _servers[0];
	}

	Configuration choseConf(Server &server) {
		Configuration conf = server._conf;
		if (server._locations.empty()) {
			if (httpParser.uri.empty()) {
				httpParser.uri = "/";
			}
			return conf;
		}
		std::string uri = httpParser.uri[httpParser.uri.length() - 1] == '/' ? httpParser.uri : httpParser.uri + "/";

		while (uri.length() > 0 && uri != "/") {
			if (server._locations.find(uri) != server._locations.end()) {
				httpParser.uri = httpParser.uri.substr(uri.length() - 1, httpParser.uri.length());
				if (httpParser.uri.empty()) {
					httpParser.uri = "/";
				}
				return server._locations[uri]._conf;
			}
			if (uri[uri.length() - 1] == '/') {
				uri = uri.substr(0, uri.length() - 2);
			}
			for (int i = uri.length() - 1; i >= 0; --i) {
				if (uri[i] == '/') {
					uri = uri.substr(0, i + 1);
					break;
				}
			}
		}

		if (server._locations.find(uri) != server._locations.end()) {
			httpParser.uri = httpParser.uri.substr(uri.length() - 1, httpParser.uri.length());
			return server._locations[uri]._conf;
		}
		if (httpParser.uri.empty()) {
			httpParser.uri = "/";
		}
		return conf;
	}

	bool exist(std::string path) {
		path = "." + path;
		std::ifstream file(path.c_str());
		bool ans = file.good();
		file.close();
		return ans;
	}

	bool is_directory(std::string path) {
		path = "." + path;
		struct stat buffer;

		bool ans = false;
		if (stat(path.c_str(), &buffer) == 0) {
			if (S_ISREG(buffer.st_mode)) {
				ans = false;
			} else {
				ans = true;
			}
		}
		return ans;
	}

	std::string	getPath(int code, const Configuration &conf) {
		if (code / 100 == 2) {
			if (is_directory(conf._root + httpParser.uri))
				return conf._root + httpParser.uri + conf._index;
			return conf._root + httpParser.uri;
		}
		if (conf._error_pages.find(code) != conf._error_pages.end()) {
			return (*conf._error_pages.find(code)).second;
		} else {
			std::stringstream ss;
			ss << code;
			std::string path("/error_pages/");
			return path + ss.str() + ".html";
		}
	}

	// Функция для получения времени создания файла в формате временной метки
	std::string get_file_creation_date(const char* file_path) {
		struct stat attr;
		if (stat(file_path, &attr) != 0) {
			return "Unknown";
		}
		char date_buff[80];
		strftime(date_buff, sizeof(date_buff), "%Y-%m-%d %H:%M:%S", localtime(&attr.st_mtime));
		return std::string(date_buff);
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
			std::string create_time = get_file_creation_date(filename.c_str());
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
		if (send(sock, response.c_str(), response.length(), 0) <= 0) {
			delete_client(sock);
			FD_CLR(sock, &_active_set);
			close(sock);
		}
	}

	std::string saveContent(std::string root) {
		time_t now = time(NULL);
		struct tm *t = localtime(&now);
		char buf[80];
		std::strftime(buf, sizeof(buf), "%Y-%m-%d_%H:%M:%S", t);
		std::stringstream ss;
		ss << buf;
		std::string time_str = ss.str();
		std::ofstream fout;
		std::string path("." + root + "/uploads/" + time_str);
		fout.open(path.c_str());
		fout << httpParser.body;
		fout.close();
		return "Location: " + root + "/uploads/" + time_str;
	}

	std::string intToStr(int num) {
		std::stringstream ss;
		ss << num;
		return ss.str();
	}

	void	executeScript(std::string uri, int sock) {
		uri = "." + uri;
		int pid = fork();
		if (pid == 0) {
			dup2(sock, STDOUT_FILENO);
			if (uri.substr(uri.length() - 3, 3) == ".py") {
				std::string python_path;
				std::vector<std::string>::iterator it;
				for (it = _paths.begin(); it != _paths.end(); ++it) {
					if (!access(((*it) + "/python3").c_str(), X_OK)) {
						python_path = (*it) + "/python3";
						break;
					}
				}
				if (it != _paths.end()) {
					char* argv[] = {const_cast<char*>(python_path.c_str()), const_cast<char*>(uri.c_str()), NULL};
					execve(python_path.c_str(), argv, vectorToCharArray(_env));
				}
			} else {
				
				execve(uri.c_str(), NULL, NULL);
			}
		}
		wait(NULL);
	}

	
};