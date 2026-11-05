/**
 * @file ServerConnections.cpp
 * @brief Manage client connections, disconnections, and related channel cleanup.
 */

#include "Server.hpp"
#include "Util.hpp"
#include <algorithm>
#include <cerrno>
#include <fcntl.h>
#include <sys/socket.h>
#include <unistd.h>

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
	m_clientRegistry[clientFd] = client;

	pollfd clientPoll = {};
	clientPoll.fd = clientFd;
	clientPoll.events = POLLIN; // Monitor incoming client data.
	m_pollFds.push_back(clientPoll);
}

void Server::disconnectClient(int fd)
{
	// Ignore unknown or already disconnected clients to avoid closing the same FD twice.
	if (m_clientRegistry.find(fd) == m_clientRegistry.end())
		return;

	// Close the client socket and remove its client record and poll() entry.
	close(fd);
	m_clientRegistry.erase(fd);
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
	for (std::map<std::string, Channel>::iterator channelRegistryIt = m_channelRegistry.begin();
		channelRegistryIt != m_channelRegistry.end(); )
	{
		Channel& channel = channelRegistryIt->second;

		// remove() moves retained elements to the front and returns their new end, without resizing.
		// erase() then deletes the remaining tail, actually removing the client's FD.
		channel.members.erase(std::remove(channel.members.begin(), channel.members.end(), fd), channel.members.end());
		channel.operators.erase(std::remove(channel.operators.begin(), channel.operators.end(), fd), channel.operators.end());
		channel.invited.erase(std::remove(channel.invited.begin(), channel.invited.end(), fd), channel.invited.end());

		// Post-increment passes the old iterator to erase() and advances it before deletion.
		// Otherwise, advance normally.
		// Incrementing an erased iterator would be undefined behavior.
		if (channel.members.empty())
			m_channelRegistry.erase(channelRegistryIt++);
		else
			++channelRegistryIt;
	}
}
