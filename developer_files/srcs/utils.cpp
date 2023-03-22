#include "./../includes/headers.hpp"
#include <cstring>

std::vector<std::string> getPathFromEnv(char **envp) {
    std::vector<std::string> pathVec;
    char *path = NULL;
    char *envVar = *envp;
    while (envVar != NULL) {
        if (std::strncmp(envVar, "PATH=", 5) == 0) {
            path = envVar + 5;
            break;
        }
        ++envp;
        envVar = *envp;
    }
    if (path != NULL) {
        char *tok = std::strtok(path, ":");
        while (tok != NULL) {
            pathVec.push_back(tok);
            tok = std::strtok(NULL, ":");
        }
    }
    return pathVec;
}

std::vector<std::string> envpToVector(char** envp) {
    std::vector<std::string> envVector;
    for (int i = 0; envp[i] != NULL; ++i) {
        envVector.push_back(std::string(envp[i]));
    }
    return envVector;
}

void parseURI(std::vector<std::string>& envp, std::string& uri) {
    std::string::size_type pos = uri.find('?');
    if (pos != std::string::npos) {
        std::string query_string = uri.substr(pos + 1);
        for (std::vector<std::string>::iterator it = envp.begin(); it != envp.end(); ++it) {
            if ((*it).compare(0, 13, "QUERY_STRING=") == 0){
                (*it) = "QUERY_STRING=" + query_string;
                uri.erase(pos);
                return;
            }
        }
        envp.push_back("QUERY_STRING=" + query_string);
        uri.erase(pos);
    }
}


// Функция, которая принимает вектор строк и возвращает массив указателей на символы
char** vectorToCharArray(std::vector<std::string>& vec)
{
    // Создаем массив указателей на символы размером vec.size()
    char** charArray = new char*[vec.size() + 1];

    // Проходимся по каждой строке вектора и добавляем ее в массив указателей на символы
    for (size_t i = 0; i < vec.size(); i++) {
        charArray[i] = &vec[i][0];
    }

    charArray[vec.size()] = NULL;

    return charArray;
}


