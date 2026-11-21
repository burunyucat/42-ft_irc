/**
 * @file main.cpp
 * @brief Read command-line arguments and start the IRC server.
 */

#include "Server.hpp"
#include "Util.hpp"

#include <exception>
#include <string>

/** @brief Parse a decimal port in the range 1 to 65535 without overflow. */
static bool parsePort(const std::string& argument, int& port);

int main(int ac, char** av)
{
	if (ac != 3)
	{
		Util::sendErrorMsg("Two arguments needed");
		return 1;
	}
	int port;
	if (!parsePort(av[1], port))
	{
		Util::sendErrorMsg("Invalid port: expected digits only and a value from 1 to 65535");
		return 1;
	}
	std::string password(av[2]);
	if (password.empty())
	{
		Util::sendErrorMsg("Invalid password: must not be empty");
		return 1;
	}

	// Last resort: a failure the event loop could not recover from (e.g. out of
	// memory outside client handling) stops the server cleanly, closing every
	// socket through the Server destructor, instead of aborting.
	try
	{
		Server srv(port, password);
		srv.run();
	}
	catch (const std::exception& e)
	{
		Util::sendErrorMsg(std::string("Server stopped: ") + e.what());
		return 1;
	}

	return 0;
}

static bool parsePort(const std::string& argument, int& port)
{
	if (argument.empty())
		return false;

	int value = 0;
	for (size_t i = 0; i < argument.size(); ++i)
	{
		if (argument[i] < '0' || argument[i] > '9')
			return false;

		const int digit = argument[i] - '0';
		// Append the digit: 666 followed by 7 becomes 6667.
		value = value * 10 + digit;
		if (value > 65535)
			return false;
	}
	if (value == 0)
		return false;

	port = value;
	return true;
}
