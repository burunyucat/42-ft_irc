/**
 * @file Util.cpp
 * @brief Implement shared whitespace and error-reporting utilities.
 */

#include "Util.hpp"
#include <iostream>
#include <string>

const std::string& Util::getWhiteSpaces()
{
	static const std::string whiteSpaces = " \t\n\v\f\r";
	return whiteSpaces;
}

void Util::sendErrorMsg(const std::string& msg)
{
	std::cerr << BG_RED << "ERROR:" << RESET << B_RED << " " << msg << RESET << std::endl;
}
