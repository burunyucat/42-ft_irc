/**
 * @file Server.cpp
 * @brief Manage server lifetime, listening socket setup, and the main event loop.
 */

#include "Server.hpp"
#include "Signal.hpp"
#include "Util.hpp"
#include <cerrno>
#include <cstddef>
#include <fcntl.h>
#include <netinet/in.h>
#include <sys/poll.h>
#include <sys/socket.h>
#include <sys/types.h>
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

void Server::updatePollEvents()
{
	// Monitor write readiness only when a client has pending output.
	for (size_t i = 0; i < m_pollFds.size(); ++i)
	{
		if (m_pollFds[i].fd == m_listenFd)
			continue;

		std::map<int, Client>::const_iterator client = m_clients.find(m_pollFds[i].fd);

		if (client == m_clients.end())
			continue;

		if (!client->second.outputBuffer.empty())
			m_pollFds[i].events |= POLLOUT;
		else
			m_pollFds[i].events &= ~POLLOUT;
	}
}

void Server::run()
{
	// Handle SIGINT and SIGTERM by requesting a clean shutdown.
	if (!Signal::installHandlers())
	{
		Util::sendErrorMsg("Failed to install signal handlers");
		return;
	}

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

	// Set non blocking mode
	// F_SETFL: sets file status flags
	// O_NONBLOCK: prevents waiting for I/O readiness.
	if (fcntl(m_listenFd, F_SETFL, O_NONBLOCK) == -1)
	{
		Util::sendErrorMsg("Failed to set listener socket non blocking mode");
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

	while (!Signal::stopRequested)
	{
		updatePollEvents();

		// Only process sockets monitored by poll(); newly added clients wait for the next poll().
		const size_t polledCount = m_pollFds.size();

		// A finite timeout also handles SIGINT received just before poll() starts.
		if (poll(&m_pollFds[0], polledCount, 200) == -1)
		{
			if (Signal::stopRequested)
				break;
			if (errno == EINTR)
				continue;
			Util::sendErrorMsg("Failed to poll sockets");
			close(m_listenFd);
			m_listenFd = -1;
			return;
		}

		// Check revents for every monitored descriptor to identify which socket triggered which events.
		// Then execute the appropriate code.
		std::vector<int> disconnectedFds;
		for (size_t i = 0; i < polledCount; ++i)
		{
			if (m_pollFds[i].revents & POLLIN)
			{
				if (m_pollFds[i].fd == m_listenFd) // New TCP connection requested
				{
					acceptClient();
				}
				else // Client socket reception buffer ready to be read
				{
					int clientFd = m_pollFds[i].fd;
					
					char buffer[2048];
					ssize_t bytesRead = recv(clientFd, buffer, sizeof(buffer), 0); 
					if (bytesRead > 0) // Data received
					{
						processClientInput(m_clients[clientFd], buffer, bytesRead);
					}
					else // Connection closed (0) or receive failure (-1): disconnect the client.
					{
						if (bytesRead == -1)
							Util::sendErrorMsg("Failed to receive client data. Client will be disconnected.");
						// Defer removal to avoid shifting descriptors during this loop.
						disconnectedFds.push_back(clientFd);
						continue;
					}
				}
			}
			if (m_pollFds[i].revents & POLLOUT)
			{
				int clientFd = m_pollFds[i].fd;
				std::map<int, Client>::iterator client = m_clients.find(clientFd);
				if (client == m_clients.end())
					continue;

				if (!processClientOutput(client->second))
				{
					disconnectedFds.push_back(clientFd);
					continue;
				}
			}
		}

		// Remove disconnected clients before the next poll().
		for (size_t i = 0; i < disconnectedFds.size(); ++i)
			disconnectClient(disconnectedFds[i]);
	}
}
