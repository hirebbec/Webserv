#include "Server.hpp"

class ConfigParser {
public:
	Server	parse_server(const std::vector<std::string> config, size_t *line_count);
	std::vector<std::string>	ft_split(const std::string &str, const std::string &charset);
	std::string	read_a_file(std::fstream &file);

private:
	Location    parse_location(const std::vector<std::string> &config, size_t *line_count);
};