/**
 * @file ServerCommands.cpp
 * @brief Parse and execute IRC commands and prepare quit notifications.
 */

#include "Server.hpp"

bool Server::parseCommand(const std::string& line, Command& newCommand)
{
	(void)line;
	(void)newCommand;
	// TODO: parse the IRC line and populate newCommand.
	return true;
}

bool Server::executeCommand(Client& client, const Command& command)
{
	if (command.name == "QUIT")
		handleQuit(client, command);
	// TODO: dispatch the remaining commands to their handlers.
	return true;
}

void Server::handleQuit(Client& client, const Command& command)
{
	if (!command.params.empty())
		client.quitReason = command.params[0];
	client.disconnectRequested = true;
}

void Server::notifyClientQuit(const Client& client)
{
	(void)client;
	// TODO: queue QUIT once per shared-channel peer, excluding the departing client.
	// Use client.quitReason; client identity and channel memberships are still available.
}
