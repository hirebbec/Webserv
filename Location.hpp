#include <map>
#include <string>
#include <vector>
#include <set>

struct return_type {
	int			code;
	std::string	location;

	return_type(): code(0), location("") {};
};

struct limit_except_type {
	std::map<std::string, bool> methods;

	limit_except_type() {
		methods.insert(std::make_pair("GET", true));
		methods.insert(std::make_pair("POST", true));
		methods.insert(std::make_pair("DELETE", true));
	}
};

struct Configuration
{
    limit_except_type				_limit_except;
    std::pair<bool, return_type>	_return;
	bool							_autoindex;
	std::string						_root;
	std::pair<bool, std::string>	_index;
	size_t							_client_max_boby_size;
	std::map<int, std::string>		_error_pages;

	Configuration(): _return(std::pair<bool, return_type>(false, return_type())), \
	_autoindex(false), _root(""), _index(std::pair<bool, std::string>(false, "")), \
	_client_max_boby_size(0) {};
};


class Location {
public:
	std::string		_uri;
	Configuration	_conf;

	Location(): _uri(""), _conf(Configuration()) {};
};