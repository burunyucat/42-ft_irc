#include "Server.hpp"

void Server::handlePing(Client& client, const Command& command)
{
	if (command.params.empty() == true || command.params[0] == "")
	{
		sendNumericAnswer(client, "409", ":No origin specified");
		return;
	}
	rawMessage(client, ":ircserv PONG ircserv :" + command.params[0]);
}
