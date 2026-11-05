/**
 * @file Command.hpp
 * @brief Define the name and parameters of a parsed IRC command.
 */

#pragma once

#include <string>
#include <vector>

struct Command
{
	std::string name;
	std::vector<std::string> params;
};
