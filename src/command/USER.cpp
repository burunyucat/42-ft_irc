#include "Server.hpp"

void Server::handleUser(void)
{
	m_clients[m_listenFd].username = command.params[0];
}