#include <iostream>
#include <fstream>

int main() {
    std::ifstream file("info.html");
    std::cout << file.is_open() << std::endl;
}