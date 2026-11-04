/**
 * @file main.cpp
 * @brief Read command-line arguments and start the IRC server.
 */

#include "Server.hpp"
#include "Util.hpp"

#include <cstdlib>
#include <string>

int main(int ac, char** av)
{
	if (ac != 3)
	{
		Util::sendErrorMsg("Two arguments needed");
		return 1;
	}
	int port(atoi(av[1]));
	std::string password(av[2]);

	Server srv(port, password);

	srv.run();

    return 0;
}
