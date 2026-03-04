#pragma one

#include <iostream>
#include "types.hpp"
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <algorithm>

std::vector<std::string> split(const std::string& s, char delim) {
    std::vector<std::string> parts;
    std::stringstream ss(s);
    std::string item;

    while (std::getline(ss, item, delim)) {
        parts.push_back(item);
    }
    return parts;
}

std::vector<std::string> loadFile(std::string filename, char delim='g'){
	std::ifstream file(filename);
	std::stringstream buffer;
	buffer << file.rdbuf();
	std::string content = buffer.str();

	content.erase(std::remove(content.begin(), content.end(), '\n'), content.end()); 
	content.erase(std::remove(content.begin(), content.end(), '\r'), content.end());

	return split(content, delim);
}
