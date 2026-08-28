
#include <iostream>
#include "quickutils.hpp"
#include <string>

void qu::plog(std::string ansi, std::string header, std::string message) {
    std::cout << ansi << "[" << header << "]\x1b[0m" << message << "\x1b[0m\n";
    return;
};