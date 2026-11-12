#include "Server.hpp"

void Server::handleUser(Client& client, const Command& command)
{
	if (client.registered == true)
	{
		sendNumericAnswer(client, "462", ":You may not reregister");
		return;
	}
	if (command.params.size() != 4)
	{
		sendNumericAnswer(client, "461", command.name + " :Not enough parameters");
		return;
	}
	client.username = command.params[0];
}
