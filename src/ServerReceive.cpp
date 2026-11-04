/**
 * @file ServerReceive.cpp
 * @brief Buffer received client data and extract complete IRC lines.
 */

#include "Server.hpp"
#include <iostream>


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

		// TODO: parse the line, then execute it for this client.
		std::cout << line << std::endl;
	}
}
