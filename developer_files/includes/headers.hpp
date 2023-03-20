#include "Webserv.hpp"

std::vector<std::string> getPathFromEnv(char **envp);
std::vector<std::string> envpToVector(char** envp);
char** createEnv(const std::vector<std::string>& env, const std::string& uri);