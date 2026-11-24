#include "Server.hpp"

void Server::handleQuit(Client& client, const Command& command)
{
	if (!command.params.empty())
		client.quitReason = command.params[0];
	client.disconnectRequested = true;
}

void Server::notifyClientQuit(const Client& client)
{
	std::string message = fullName(client) + " QUIT :" + client.quitReason;
	notifyPeers(client, message);
}
