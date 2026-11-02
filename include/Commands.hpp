#pragma once

#include <string>
#include <iostream>
#include <vector>

struct Commands
{
	std::string name;
	std::vector<std::string> params;
};
int parsingCommand(std::string line);