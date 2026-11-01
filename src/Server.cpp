#include "Server.hpp"

Server::Server(int port, const std::string& password)
	// Initial server configuration
	: m_listenFd(-1), m_port(port), m_password(password)
{

}
