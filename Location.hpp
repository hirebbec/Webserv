#include <map>
#include <string>
#include <vector>
#include <set>

struct return_type {
	int			code;
	std::string	location;
};

struct limit_except_type {
	bool	GET;
	bool	POST;
	bool	DELETE;

	limit_except_type(bool get = true, bool post = true, bool delet = true):
	GET(get), POST(post), DELETE(delet) {};
};

struct Location
{
    limit_except_type				_limit_except;
    std::pair<bool, return_type>	_return;
	std::map<int, std::string>		_error_pages;
	bool							_autoindex;
	std::string						_root;
	std::string						_index;
	size_t							_client_max_boby_size;
};
