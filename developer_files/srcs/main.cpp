#include "./../includes/headers.hpp"

int main(int argc, char** argv, char** envp) {
	Webserv webserv(envpToVector(envp), getPathFromEnv(envp));
	ConfigParser _config_parser;

	if (argc > 2) {
		std::cerr << "Usage: " << argv[0] << " <config_file>" << std::endl;
		return 1;
	}
	std::string file_name;
	if (argc == 1) {
		file_name = "./configs/conf_test.conf";
	} else {
		file_name = argv[1];
	}

	std::fstream				file;
	std::string					str;
	std::vector<std::string>	split_conf;

	file.open(file_name.c_str());
	if (!file.is_open()) {
		std::cerr << "File does not exist or can not be open" << std::endl;
		exit(1);
	}

	str = _config_parser.read_a_file(file);
	file.close();
	split_conf = _config_parser.ft_split(str, "\n");

	size_t line_count = 0;
	while (line_count < split_conf.size())
	{
		std::vector<std::string> line = _config_parser.ft_split(split_conf[line_count], " \t;");
		if (line[0] == "server")
		{
			Server  server = _config_parser.parse_server(split_conf, &line_count);
			webserv.addServer(server);
		}
		else
		{
			std::cerr << "server is missing in config file";
			exit(1);
		}
		line_count++;
	}

	if (webserv.init())
		webserv.startMonitoring();

	return 0;
}