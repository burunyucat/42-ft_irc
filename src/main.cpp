#include "Server.hpp"

#include <cstdlib>
#include <string>

int main(int ac, char** av)
{
	(void)ac;
	(void)av;

	

	int port(atoi(av[0]));
	std::string password(av[2]);

	Server srv(port, password);

	srv.run();

    return 0;
}
