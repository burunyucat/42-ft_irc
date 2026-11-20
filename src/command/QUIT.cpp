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

// nc -C localhost 6667
// PASS motdepasse
// NICK alice
// USER alice 0 * :Alice
// JOIN #test

// nc -C localhost 6667
// PASS motdepasse
// NICK bob
// USER bob 0 * :Bob
// JOIN #test

// irssi -c localhost -p 6667 -w pw -n alice
// irssi -c localhost -p 6667 -w pw -n bob
