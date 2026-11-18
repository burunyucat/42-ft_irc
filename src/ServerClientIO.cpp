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

	// Clears a continuous line that is too long to be used and should launch a 417 error.
	if (client.discardingLine == true)
	{
		size_t pos_2 = client.inputBuffer.find('\n');
		if (pos_2 == std::string::npos)
		{
			client.inputBuffer.erase();
			return ;
		}
		else
		{
			client.inputBuffer.erase(0, pos_2 + 1);
			client.discardingLine = false;
		}
	}

	size_t pos;
	while ((pos = client.inputBuffer.find('\n')) != std::string::npos)
	{
		// Extract the line without its delimiter.
		std::string line = client.inputBuffer.substr(0, pos);
		if (!line.empty() && line[line.size() - 1] == '\r')
			line.erase(line.size() - 1, 1);
		if (line.size() > LIMIT_LINE)
		{
			sendNumericAnswer(client, "417", ":Input line was too long");
			client.inputBuffer.erase(0, pos + 1);
			continue;
		}
		// Remove the complete line and keep any incomplete data.
		client.inputBuffer.erase(0, pos + 1);
		Command command;
		if (!parseCommand(line, command))
		{
			Util::sendErrorMsg("Failed to parse command");
			continue;
		}
		executeCommand(client, command);
		// Stop processing further commands when the client requests disconnection.
		if (client.disconnectRequested)
			return;
	}
	if (client.inputBuffer.size() > LIMIT_LINE + 2)
	{
		if (client.discardingLine == false)
			sendNumericAnswer(client, "417", ":Input line was too long");
		client.inputBuffer.erase();
		client.discardingLine = true;
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
