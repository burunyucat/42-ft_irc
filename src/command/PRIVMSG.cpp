#include "Server.hpp"

void Server::handlePrivmsg(Client& client, const Command& command)
{
	if (command.params.empty() || command.params[0] == "")
	{
		sendNumericAnswer(client, "411", ":No recipient given (PRIVMSG)");
		return;
	}
	if (command.params.size() < 2 || command.params[1] == "")
	{
		sendNumericAnswer(client, "412", ":No text to send");
		return;
	}
	std::string msg = fullName(client) + " " + command.name + " " + command.params[0] + " :";
	for (size_t i = 1; i < command.params.size(); i++)
	{
		msg.append(command.params[i]);
		if (i != command.params.size() - 1)
			msg.append(" ");
	}
	if (command.params[0][0] == '#')
	{
		Channel *dest = findChannel(command.params[0]);
		if (!dest)
		{
			sendNumericAnswer(client, "403", command.params[0] + " :No such channel");
			return;
		}
		else if (!isMember(client.fd, *dest))
		{
			sendNumericAnswer(client, "404", command.params[0] + " :Cannot send to channel");
			return;
		}
		broadcast(*dest, msg, client.fd);
	}
	else
	{
		Client *dest_f = findClientByNick(command.params[0]);
		if (!dest_f || !dest_f->registered)
		{
			sendNumericAnswer(client, "401", command.params[0] + " :No such nick/channel");
			return;
		}
		rawMessage(*dest_f, msg);
	}
}
