/**
 * @file ServerCommands.cpp
 * @brief Parse and execute IRC commands and prepare quit notifications.
 */

#include "Server.hpp"
#include <cctype>

bool Server::parseCommand(const std::string& line, Command& newCommand)
{
	size_t	pos = line.find_first_not_of(" ", 0);
	if (pos == line.npos)
		return false;
	if (line[pos] == ':')
	{
		pos = line.find(" ", pos);
		if (pos == line.npos)
			return false;
		pos = line.find_first_not_of(" ", pos);
		if (pos == line.npos)
			return false;
	}
	size_t	end = line.find(" ", pos);
	if (end != line.npos)
	{
		newCommand.name = line.substr(pos, end - pos);
		for (size_t i = 0; i < newCommand.name.size(); i++)
			newCommand.name[i] = std::toupper(static_cast<unsigned char>(newCommand.name[i]));
	}
	else
	{
		newCommand.name = line.substr(pos);
		for (size_t i = 0; i < newCommand.name.size(); i++)
			newCommand.name[i] = std::toupper(static_cast<unsigned char>(newCommand.name[i]));
		return true;
	}
	pos = end;
	while (pos != line.npos)
	{
		pos = line.find_first_not_of(" ", pos);
		if (pos == line.npos)
			break;
		if (line[pos] == ':')
		{
			newCommand.params.push_back(line.substr(pos + 1));
			return true;
		}
		end = line.find(" ", pos);
		if (end != line.npos)
			newCommand.params.push_back(line.substr(pos, end - pos));
		else
			newCommand.params.push_back(line.substr(pos));
		pos = end;
	}
	return true;
}

void Server::executeCommand(Client& client, const Command& command)
{
	if (command.name == "QUIT")
		handleQuit(client, command);
	// TODO: dispatch the remaining commands to their handlers.
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
