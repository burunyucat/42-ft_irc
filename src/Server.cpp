#include "Server.hpp"
#include "Util.hpp"
#include <cstddef>
#include <fcntl.h>
#include <iostream>
#include <netinet/in.h>
#include <sys/poll.h>
#include <sys/socket.h>
#include <unistd.h>

Server::Server(int port, const std::string& password)
	// Initial server configuration
	: m_listenFd(-1), m_port(port), m_password(password)
{

}

Server::~Server()
{
	// Close all connected client sockets; the map key is the socket descriptor.
	for (std::map<int, Client>::iterator it = m_clients.begin();
			it != m_clients.end();
			++it)
	{
		close(it->first);
	}

	if (m_listenFd != -1)
		close(m_listenFd);
}
