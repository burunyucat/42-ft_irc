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

void Server::run()
{
	// Create the listener socket with TCP protocol
	// AF_INET: IPv4
	// SOCK_STREAM: connection-oriented byte stream
	// IPPROTO_TCP: TCP protocol.
	m_listenFd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (m_listenFd == -1)
	{
		Util::sendErrorMsg("Failed to create listener socket");
		return;
	}

	// Enable non blocking mode
	// F_SETFL: sets file status flags
	// O_NONBLOCK: prevents waiting for I/O readiness.
	if (fcntl(m_listenFd, F_SETFL, O_NONBLOCK) == -1)
	{
		Util::sendErrorMsg("Failed to set non blocking mode to listener socket");
		return;
	}

	// sockaddr_in holds the IPv4 address and port
	// cast its pointer to the generic sockaddr type expected by bind().
	sockaddr_in address = {};
	address.sin_family = AF_INET; // IPv4 address family, matching the socket.
	address.sin_port = htons(m_port);
	address.sin_addr.s_addr = htonl(INADDR_LOOPBACK); // 127.0.0.1: local connections only.
	const sockaddr* sockaddr_ptr = reinterpret_cast<const sockaddr*>(&address);

	// Binding port and loopback address to listener socket
	if (bind(m_listenFd,
		sockaddr_ptr,
		sizeof(address)) == -1)
	{
		Util::sendErrorMsg("Failed to bind listener socket");
		close(m_listenFd);
		m_listenFd = -1;
		return;
	}

	// Start listening for incoming connections
	// SOMAXCONN: backlog size for connections waiting for accept(), capped by the OS.
	if (listen(m_listenFd, SOMAXCONN) == -1)
	{
		Util::sendErrorMsg("Failed to listen on listener socket");
		close(m_listenFd);
		m_listenFd = -1;
		return;
	}

	// create the listener socket pollfd

	// pollfd fields:
	// fd: socket descriptor to monitor.
	// events: requested events (e.g. POLLIN, POLLOUT).
	// revents: events detected and filled in by poll().
	pollfd listener = {};
	listener.fd = m_listenFd;
	listener.events = POLLIN; // Notify when a connection is ready to be accepted.
	m_pollFds.push_back(listener);

	// -1: wait indefinitely for an event on the monitored sockets.
	if (poll(&m_pollFds[0], m_pollFds.size(), -1) == -1)
	{
		Util::sendErrorMsg("Failed to poll sockets");
		close(m_listenFd);
		m_listenFd = -1;
		return;
	}

	// Check revents for every monitored descriptor to identify which socket triggered which events.
	// Then execute the appropriate code.
	for (size_t i = 0; i < m_pollFds.size(); ++i)
	{
		if (m_pollFds[i].revents & POLLIN)
		{
			if (m_pollFds[i].fd == m_listenFd)
				std::cout << "Demande de connexion au socket d'ecoute" << std::endl;
		}
	}
}
