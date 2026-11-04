#include "Server.hpp"
#include "Signal.hpp"
#include "Util.hpp"
#include <algorithm>
#include <cerrno>
#include <cstddef>
#include <fcntl.h>
#include <iostream>
#include <netinet/in.h>
#include <ostream>
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

void Server::disconnectClient(int fd)
{
	// Ignore unknown or already disconnected clients to avoid closing the same FD twice.
	if (m_clients.find(fd) == m_clients.end())
		return;

	// Close the client socket and remove its client record and poll() entry.
	close(fd);
	m_clients.erase(fd);
	for (std::vector<pollfd>::iterator it = m_pollFds.begin();
			it != m_pollFds.end();
			++it)
	{
		if (it->fd == fd)
		{
			m_pollFds.erase(it);
			break;
		}
	}

	// Remove the client's references from every channel and delete channels left empty.
	for (std::map<std::string, Channel>::iterator it = m_channels.begin();
		it != m_channels.end(); )
	{
		Channel& channel = it->second;

		// remove() moves retained elements to the front and returns their new end, without resizing.
		// erase() then deletes the remaining tail, actually removing the client's FD.
		channel.members.erase(std::remove(channel.members.begin(), channel.members.end(), fd), channel.members.end());
		channel.operators.erase(std::remove(channel.operators.begin(), channel.operators.end(), fd), channel.operators.end());
		channel.invited.erase(std::remove(channel.invited.begin(), channel.invited.end(), fd), channel.invited.end());

		// it++ passes the old iterator to erase() and advances it before that element is deleted.
		// Otherwise, advance normally.
		// Incrementing an erased iterator would be undefined behavior.
		if (channel.members.empty())
			m_channels.erase(it++);
		else
			++it;
	}
}

void Server::processClientInput(Client& client, const char* buffer, size_t bytesRead)
{
	client.inputBuffer.append(buffer, bytesRead);

	size_t pos;
	while ((pos = client.inputBuffer.find("\r\n")) != std::string::npos)
	{
		// Extract the line without its delimiter.
		std::string line = client.inputBuffer.substr(0, pos);

		// Remove the complete line and keep any incomplete data.
		client.inputBuffer.erase(0, pos + 2);

		// TODO: parse the line, then execute it for this client.
		std::cout << line << std::endl;
	}
}

void Server::acceptClient()
{
	int clientFd = accept(m_listenFd, NULL, NULL);
	if (clientFd == -1)
	{
		// EAGAIN / EWOULDBLOCK: no connection available anymore.
		if (errno != EAGAIN && errno != EWOULDBLOCK)
			Util::sendErrorMsg("Failed to accept client");
		return;
	}

	// Set non blocking mode on client socket.
	if (fcntl(clientFd, F_SETFL, O_NONBLOCK) == -1)
	{
		Util::sendErrorMsg("Failed to set client socket non blocking");
		close(clientFd);
		return;
	}

	Client client;
	client.fd = clientFd;
	m_clients[clientFd] = client;

	pollfd clientPoll = {};
	clientPoll.fd = clientFd;
	clientPoll.events = POLLIN; // Monitor incoming client data.
	m_pollFds.push_back(clientPoll);
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
					}
				}
			}
		}

		// Remove disconnected clients before the next poll().
		for (size_t i = 0; i < disconnectedFds.size(); ++i)
			disconnectClient(disconnectedFds[i]);
	}
}
