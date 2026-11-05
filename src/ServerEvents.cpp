/**
 * @file ServerEvents.cpp
 * @brief Configure and handle poll events for listening and client sockets.
 */

#include "Server.hpp"
#include "Util.hpp"
#include <cstddef>
#include <sys/socket.h>
#include <sys/types.h>

void Server::updateClientPolloutFlags()
{
	// Monitor write readiness only when a client has pending output.
	for (size_t i = 0; i < m_pollFds.size(); ++i)
	{
		if (m_pollFds[i].fd == m_listenFd)
			continue;

		std::map<int, Client>::const_iterator clientRegistryIt = m_clientRegistry.find(m_pollFds[i].fd);

		if (clientRegistryIt == m_clientRegistry.end())
			continue;

		if (!clientRegistryIt->second.outputBuffer.empty())
			m_pollFds[i].events |= POLLOUT;
		else
			m_pollFds[i].events &= ~POLLOUT;
	}
}

bool Server::handleListenerEvents(short revents)
{
	if (revents & POLLHUP)
	{
		Util::sendErrorMsg("Listener socket hung up. Server will stop.");
		return false;
	}

	if (revents & POLLERR)
	{
		Util::sendErrorMsg("Listener socket failed. Server will stop.");
		return false;
	}

	if (revents & POLLIN)
		acceptClient();

	return true;
}

bool Server::handleClientEvents(int clientFd, short revents)
{
	std::map<int, Client>::iterator clientRegistryIt = m_clientRegistry.find(clientFd);
	if (clientRegistryIt == m_clientRegistry.end())
		return true;

	if (clientRegistryIt->second.disconnectRequested)
		return false;

	if (revents & POLLERR)
	{
		Util::sendErrorMsg("Client socket failed. Client will be disconnected.");
		return false;
	}

	// Drain remaining client data across iterations before disconnecting.
	if (revents & (POLLIN | POLLHUP))
	{
		char buffer[2048];
		ssize_t bytesRead = recv(clientFd, buffer, sizeof(buffer), 0);
		if (bytesRead > 0) // Data received
		{
			processClientInput(clientRegistryIt->second, buffer, bytesRead);
			if (clientRegistryIt->second.disconnectRequested)
				return false;
		}
		else // Connection closed (0) or receive failure (-1): disconnect the client.
		{
			if (bytesRead == -1)
				Util::sendErrorMsg("Failed to receive client data. Client will be disconnected.");
			return false;
		}
	}

	if (revents & POLLOUT)
	{
		if (!processClientOutput(clientRegistryIt->second))
			return false;
	}

	return true;
}
