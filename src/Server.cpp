/**
 * @file Server.cpp
 * @brief Manage server lifetime, listening socket setup, and the main event loop.
 */

#include "Server.hpp"
#include "Signal.hpp"
#include "Util.hpp"
#include "Channel.hpp"
#include <cerrno>
#include <cstddef>
#include <fcntl.h>
#include <netinet/in.h>
#include <sys/poll.h>
#include <sys/socket.h>
#include <unistd.h>


Server::Server(int port, const std::string& password)
	// Initial server configuration
	: m_listenFd(-1), m_port(port), m_password(password)
	// Handler map creation for the commands
	{
		m_commandHandler["PASS"] = &Server::handlePass;
		m_commandHandler["NICK"] = &Server::handleNick;
		m_commandHandler["USER"] = &Server::handleUser;
		m_commandHandler["JOIN"] = &Server::handleJoin;
		m_commandHandler["PRIVMSG"] = &Server::handlePrivmsg;
		m_commandHandler["INVITE"] = &Server::handleInvite;
		m_commandHandler["MODE"] = &Server::handleMode;
		m_commandHandler["KICK"] = &Server::handleKick;
		m_commandHandler["TOPIC"] = &Server::handleTopic;
		m_commandHandler["PART"] = &Server::handlePart;
		m_commandHandler["QUIT"] = &Server::handleQuit;
		m_commandHandler["PING"] = &Server::handlePing;
		m_commandHandler["PONG"] = &Server::handlePong;
		m_commandHandler["NAMES"] = &Server::handleNames;
		m_authorizedHandler.insert("PASS");
		m_authorizedHandler.insert("NICK");
		m_authorizedHandler.insert("USER");
		m_authorizedHandler.insert("PING");
		m_authorizedHandler.insert("QUIT");
	}

Server::~Server()
{
	// Close all connected client sockets; the map key is the socket descriptor.
	for (std::map<int, Client>::iterator clientRegistryIt = m_clientRegistry.begin();
			clientRegistryIt != m_clientRegistry.end();
			++clientRegistryIt)
	{
		close(clientRegistryIt->first);
	}

	if (m_listenFd != -1)
		close(m_listenFd);
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
		return;
	}

	// Start listening for incoming connections
	// SOMAXCONN: backlog size for connections waiting for accept(), capped by the OS.
	if (listen(m_listenFd, SOMAXCONN) == -1)
	{
		Util::sendErrorMsg("Failed to listen on listener socket");
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
		updateClientPolloutFlags();

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
			return;
		}

		// Check revents for every monitored descriptor to identify which socket triggered which events.
		// Then execute the appropriate code.
		std::vector<int> disconnectedFds;
		for (size_t i = 0; i < polledCount; ++i)
		{
			if (m_pollFds[i].fd == m_listenFd)
			{
				if (!handleListenerEvents(m_pollFds[i].revents))
					return;
			}
			else if (!handleClientEvents(m_pollFds[i].fd, m_pollFds[i].revents))
			{
				// Defer removal to avoid shifting descriptors during this loop.
				disconnectedFds.push_back(m_pollFds[i].fd);
			}
		}

		// Remove disconnected clients before the next poll().
		for (size_t i = 0; i < disconnectedFds.size(); ++i)
			disconnectClient(disconnectedFds[i]);
	}
}

bool Server::isNicknameInUse(const std::string &nick, int exceptFd)
{
	std::string	tested = nick;
	for (size_t j = 0; j < nick.length(); j++)
		tested[j] = tolower(nick[j]);
	for (std::map<int, Client>::iterator it = this->m_clientRegistry.begin(); it != this->m_clientRegistry.end(); ++it)
	{
		if (it->first == exceptFd)
			continue;
		std::string registered = it->second.nickname;
		for (size_t i = 0; i < it->second.nickname.length(); i++)
			registered[i] = tolower(it->second.nickname[i]);
		if (tested == registered)
			return true;
	}
	return false;
}

void Server::rawMessage(Client &client, const std::string &message)
{
	if (message.empty())
		return;
	std::string out = message + "\r\n";
	client.outputBuffer.append(out);
}

std::string Server::fullName(const Client &client) const
{
	std::string out = ":" + client.nickname + "!" + client.username + "@localhost";
	return out;
}

void Server::broadcast(const Channel &channel, const std::string &message, int exceptFd)
{
	for (size_t i = 0; i < channel.members.size(); i++)
	{
		if (channel.members[i] == exceptFd)
			continue;
		std::map<int, Client>::iterator it = this->m_clientRegistry.find(channel.members[i]);
		if (it == this->m_clientRegistry.end())
			continue;
		rawMessage(it->second, message);
	}
}

Channel *Server::findChannel(const std::string &name)
{
	std::map<std::string, Channel>::iterator it = this->m_channelRegistry.find(name);
	if (it == m_channelRegistry.end())
		return NULL;
	return &it->second;
}
