/**
 * @file ServerClientIO.cpp
 * @brief Process received IRC lines and send buffered client output.
 */

#include "Command.hpp"
#include "Server.hpp"
#include "Util.hpp"
#include <sys/socket.h>


void Server::processClientInput(Client& client, const char* buffer, size_t bytesRead)
{
	if (client.disconnectRequested)
		return;

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
			continue;
		}
		if (!executeCommand(client, command))
		{
			Util::sendErrorMsg("Failed to execute command");
		}
		// A disconnect request takes precedence over recoverable command errors.
		if (client.disconnectRequested)
			return;
	}
}

bool Server::processClientOutput(Client& client)
{
	std::string& output = client.outputBuffer;
	if (output.empty())
		return true;

	// client.fd: destination socket.
	// output.c_str(): data to send.
	// output.size(): maximum bytes to send; 0: no special send flags.
	ssize_t bytesSent = send(client.fd, output.c_str(), output.size(), 0);
	if (bytesSent <= 0)
	{
		Util::sendErrorMsg("Failed to send client data. Client will be disconnected.");
		return false;
	}

	// Keep any data that could not be sent yet.
	output.erase(0, bytesSent);
	return true;
}
