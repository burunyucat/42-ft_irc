#pragma once

#include <string>

struct Client
{
	// Connected socket fd
	int fd;

	// IRC nickname
	std::string nickname;
	// IRC username
	std::string username;
	// Real name provided by USER
	std::string realName;
	// Server password accepted
	bool authenticated;
	// IRC registration completed
	bool registered;

	std::string inputBuffer;
	std::string outputBuffer;

	// Default disconnected client
	Client() : fd(-1), authenticated(false), registered(false) {}
};
