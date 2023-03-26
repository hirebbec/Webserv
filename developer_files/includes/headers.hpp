#include "Webserv.hpp"
#include "ConfigParser.hpp"

std::vector<std::string> getPathFromEnv(char **envp);
std::vector<std::string> envpToVector(char** envp);
void parseURI(std::vector<std::string>& envp, std::string& uri);
char** vectorToCharArray(std::vector<std::string>& vec);