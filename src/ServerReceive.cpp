/**
 * @file ServerReceive.cpp
 * @brief Buffer received client data and extract complete IRC lines.
 */

#include "Command.hpp"
#include "Server.hpp"
#include "Util.hpp"


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

		Command command;
		if (!parseCommand(line, command))
		{
			Util::sendErrorMsg("Failed to parse command");
			return;
		}
		if (!executeCommand(client, command))
		{
			Util::sendErrorMsg("Failed to execute command");
			return;
		}
	}
}
