#include <iostream>
#include <fstream>
#include <vector>
#include "../includes/ConfigParser.hpp"

static bool	ft_in_charset(char const c, const std::string &charset)
{
	int	i_charset;

	i_charset = 0;
	while (charset[i_charset])
	{
		if (c == charset[i_charset++])
			return true;
	}
	return false;
}

void	print_error_end_exit(std::string msg)
{
	std::cerr << msg << std::endl;
	exit(1);
}

std::vector<std::string>	ConfigParser::ft_split(const std::string &str, const std::string &charset)
{
	std::vector<std::string>	res;
	std::string					tmp;
	size_t						i;

	i = 0;
	while (i < str.size())
	{
		while (i < str.size() && ft_in_charset(str[i], charset))
			i++;
		if (i < str.size())
		{
			tmp = "";
			while (i < str.size() && !ft_in_charset(str[i], charset))
				tmp += str[i++];
			res.push_back(tmp);
		}
	}
	return res;
}

Location    ConfigParser::parse_location(const std::vector<std::string> &config, size_t *line_count)
{
	std::vector<std::string> line = ft_split(config[*line_count], "; \t");

	if (line.size() != 3)
		print_error_end_exit("expecting a directory and '{' after 'location'");
	Location	location(line[1]);
	if (line[2] != "{")
		print_error_end_exit("expecting '{' after 'location' directory");
	(*line_count)++;
	std::vector<std::string>::const_iterator it = config.begin() + *line_count;
	while (it != config.end())
	{
		std::vector<std::string> line = ft_split(*it, " \t;");
		if (!line.size() || !line[0].size() || line[0][0] == '#')
		{
			it++;
			(*line_count)++;
			continue;
		}
		if (line[0] == "}")
			break;
		else if (line[0] == "return")
		{
			if (line.size() != 3)
				print_error_end_exit("expecting 2 arguments after 'return'");
			int type = atoi(line[1].c_str());
			if (!(type == 300 || type == 404 || type == 403))
				print_error_end_exit("redirection code is invalid");

			location._conf._return.first = true;
			location._conf._return.second.code = type;
			location._conf._return.second.location = line[2];
		}
		else if (line[0] == "root")
		{
			if (line.size() != 2)
				print_error_end_exit("expecting 1 argument after 'root'");
			std::string tmp = line[1];
			if (tmp.size() > 1 && tmp[tmp.size() - 1] == '/')
				tmp.resize(tmp.size() - 1);
			location._conf._root = tmp;
		}
		else if (line[0] == "index")
		{
			if (line.size() != 2)
				print_error_end_exit("expecting 1 argument after 'index'");
			location._conf._index = line[1];
		}
		else if (line[0] == "limit_except")
		{
			if (line.size() > 4 || line.size() == 1)
				print_error_end_exit("expecting 1, 2 or 3 argument after 'limit_except'");
			size_t i = 1;
			while (i < line.size()) {
				if (line[i] == "GET") {
					location._conf._limit_except.methods[line[i]] = false;
				} else if (line[i] == "POST") {
					location._conf._limit_except.methods[line[i]] = false;
				} else if (line[i] == "DELETE") {
					location._conf._limit_except.methods[line[i]] = false;
				} else {
					print_error_end_exit("expecting GET, POST, DELETE argument after 'limit_except', found " + line[i]);
				}
				i++;
			}
		}
		else if (line[0] == "error_pages")
		{
			if (line.size() != 3)
				print_error_end_exit("expecting 2 argument after 'error_pages'");
			location._conf._error_pages.insert(std::make_pair(atoi(line[1].c_str()), line[2]));
		}
		else if (line[0] == "autoindex")
		{
			if (line.size() != 2)
				print_error_end_exit("expecting 1 argument after 'autoindex'");
			if (line[1] == "on") {
				location._conf._autoindex = true;
			} else {
				print_error_end_exit("expecting 'on' argument after 'autoindex', found " + line[1]);
			}
		}
		else if (line[0] == "client_max_body_size")
		{
			if (line.size() != 2)
				print_error_end_exit("expecting 1 argument after 'client_max_body_size'");

			location._conf._client_max_boby_size = atoi(line[1].c_str());;
		}
		else
			std::cout << "DEBUG MESSAGE: UNEXPECTED LOCATION PROPERTY: " << line[0];
		it++;
		(*line_count)++;
	}
	std::vector<std::string> end = ft_split(config[*line_count], " \t");
	if (end[0] != "}")
		print_error_end_exit(" location: missing closing bracket");
	return (location);
}

Server	ConfigParser::parse_server(const std::vector<std::string> config, size_t *line_count)
{
	std::vector<std::string> line = ft_split(config[0], " \t;");
	if (line[1] != "{")
		print_error_end_exit(" missing { after server");
	Server  server;
	std::vector<std::string>::const_iterator it = config.begin() + *line_count;
	if (it == config.end())
		print_error_end_exit("server config is not valid");
	it++;
	(*line_count)++;
	while (it != config.end())
	{
		std::vector<std::string> line = ft_split(*it, " \t;");
		if (!line.size() || !line[0].size() || line[0][0] == '#')
		{
			it++;
			(*line_count)++;
			continue;
		}
		if (line[0] == "}")
			break;
		else if (line[0] == "location")
		{
			if (line.size() < 2)
				print_error_end_exit("expecting 2 arguments after 'location'" );
			size_t  old_line_count = *line_count;
			Location location = parse_location(config, line_count);
			server._locations.insert(std::make_pair(location._uri, location));
			it += *line_count - old_line_count;
		}
		else if (line[0] == "server_name")
		{
			if (line.size() < 2)
				print_error_end_exit("expecting at least 1 argument after 'server_name'");
			for (std::vector<std::string>::const_iterator s_name = line.begin() + 1; s_name != line.end(); s_name++)
				server._server_name.push_back(*s_name);
		}
		else if (line[0] == "listen")
		{
			if (line.size() != 2)
				print_error_end_exit("expecting 1 argument after 'listen'");
			server._port = line[1].c_str();
		}
		else if (line[0] == "root")
		{
			if (line.size() != 2)
				print_error_end_exit("expecting 1 argument after 'root'");
			std::string tmp = line[1];
			if (tmp.size() > 1 && tmp[tmp.size() - 1] == '/')
				tmp.resize(tmp.size() - 1);
			server._conf._root = tmp;
		}
		else if (line[0] == "max_client_body_size")
		{
			if (line.size() != 2)
				print_error_end_exit("expecting 1 argument after 'max_client_body_size'");
			server._conf._client_max_boby_size = atoll(line[1].c_str());
		}
		else if (line[0] == "index")
		{
			if (line.size() != 2)
				print_error_end_exit("expecting 1 argument after 'index'");
			server._conf._index = line[1];
		}
		else if (line[0] == "error_pages")
		{
			if (line.size() != 3)
				print_error_end_exit("expecting 3 arguments in 'error_page'");
			server._conf._error_pages.insert(std::pair<unsigned int, std::string>(atoi(line[1].c_str()), line[2]));
		}
		else if (line[0] == "limit_except")
		{
			if (line.size() > 4 || line.size() == 1)
				print_error_end_exit("expecting 1, 2 or 3 argument after 'limit_except'");
			size_t i = 1;
			while (i < line.size()) {
				if (line[i] == "GET") {
					server._conf._limit_except.methods[line[i]] = false;
				} else if (line[i] == "POST") {
					server._conf._limit_except.methods[line[i]] = false;
				} else if (line[i] == "DELETE") {
					server._conf._limit_except.methods[line[i]] = false;
				} else {
					print_error_end_exit("expecting GET, POST, DELETE argument after 'limit_except', found " + line[i]);
				}
				i++;
			}
		}
		else
			std::cout << "DEBUG MESSAGE: UNEXPECTED SERVER PROPERTY: " << line[0];
		it++;
		(*line_count)++;
	}
	std::vector<std::string> end = ft_split(*it, " /t");
	while (!end.size() || end[0] != "}")
	{
		(*line_count)++;
		end = ft_split(config[*line_count], " \t");
	}
	if (end.size() && end[0] != "}")
		print_error_end_exit("missing closing bracket");
	if (!server.is_valid()) {
		print_error_end_exit("Server config is not valid");
	}
	return (server);
}

std::string	ConfigParser::read_a_file(std::fstream &file)
{
	std::string	res;
	std::string	tmp;

	while (getline(file, tmp))
		res += tmp + "\n";
	return (res);
}
