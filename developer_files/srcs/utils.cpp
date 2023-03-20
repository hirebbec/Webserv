#include "./../includes/headers.hpp"

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

char** createEnv(const std::vector<std::string>& env, const std::string& uri) {
    // Count the number of environment variables in env
    int envCount = env.size();
    
    // Parse the URI to extract any environment variables it contains
    std::string envString = uri.substr(uri.find("?") + 1);
    std::vector<std::string> uriEnv;
    size_t pos = 0;
    while ((pos = envString.find("&")) != std::string::npos) {
        std::string token = envString.substr(0, pos);
        uriEnv.push_back(token);
        envString.erase(0, pos + 1);
    }
    uriEnv.push_back(envString);

    // Count the number of environment variables in uriEnv
    int uriEnvCount = uriEnv.size();

    // Allocate memory for the new environment variable array
    char** newEnv = new char*[envCount + uriEnvCount + 1];
    int i = 0;

    // Copy the environment variables from env to newEnv
    for (i = 0; i < envCount; i++) {
        const char* envStr = env[i].c_str();
        int envLen = strlen(envStr) + 1;
        newEnv[i] = new char[envLen];
        strncpy(newEnv[i], envStr, envLen);
    }

    // Copy the environment variables from uriEnv to newEnv
    for (int j = 0; j < uriEnvCount; j++) {
        const char* envStr = uriEnv[j].c_str();
        int envLen = strlen(envStr) + 1;
        newEnv[i + j] = new char[envLen];
        strncpy(newEnv[i + j], envStr, envLen);
    }

    // Set the last element of the newEnv array to NULL
    newEnv[envCount + uriEnvCount] = NULL;

    return newEnv;
}
