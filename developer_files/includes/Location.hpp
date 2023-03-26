#include <map>
#include <string>
#include <vector>
#include <set>

struct return_type {
	int			code;
	std::string	location;

	return_type(): code(0), location("") {};

	std::string	getReturnPath() {
		std::string header("Location: ");
		return header + location;
	}
};

struct limit_except_type {
	typedef std::map<std::string, bool>::iterator	iterator;

	std::map<std::string, bool> methods;

	limit_except_type() {
		methods.insert(std::make_pair("GET", true));
		methods.insert(std::make_pair("POST", true));
		methods.insert(std::make_pair("DELETE", true));
	}

	std::string	getAllowMethods() {
		std::string allowed("Allow: ");
		for (iterator it = methods.begin(); it != methods.end(); ++it) {
			if ((*it).second) {
				allowed.append((*it).first);
				allowed.append(", ");
			}
		}
		if (allowed.length() > 0) {
			allowed.erase(allowed.end() - 1);
			allowed.erase(allowed.end() - 1);
		}
		return allowed;
	}
};

struct Configuration
{
    limit_except_type				_limit_except;
    std::pair<bool, return_type>	_return;
	bool							_autoindex;
	std::string						_root;
	std::string						_index;
	size_t							_client_max_boby_size;
	std::map<int, std::string>		_error_pages;

	Configuration(): _return(std::pair<bool, return_type>(false, return_type())), \
	_autoindex(false), _root("/"), _index("index.html"),	_client_max_boby_size(4096) {};
};


class Location {
public:
	std::string		_uri;
	Configuration	_conf;

	Location(std::string uri = "", Configuration conf = Configuration()): _uri(uri), _conf(conf) {};
};