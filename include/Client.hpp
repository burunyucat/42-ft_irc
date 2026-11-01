#pragma once

#include <string>

struct Client
{
	int fd;

	std::string inputBuffer;
	std::string outputBuffer;
};
