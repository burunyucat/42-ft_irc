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
	std::string txt = "";
	for (size_t i = 1; i < command.params.size(); i++)
	{
		txt.append(command.params[i]);
		if (i != command.params.size() - 1)
			txt.append(" ");
	}
	std::vector<std::string> targets;
	splitComma(command.params[0], targets);
	for (std::vector<std::string>::iterator target = targets.begin(); target != targets.end(); ++target)
	{
		const std::string &name = *target;
		if (name.empty() == true || name == "")
			continue;
		std::string msg = fullName(client) + " " + command.name + " " + name + " :" + txt;
		if (isPrefixChannel(name[0]))
		{
			Channel *dest = findChannel(name);
			if (!dest)
			{
				sendNumericAnswer(client, "403", name + " :No such channel");
				continue;
			}
			else if (!isMember(client.fd, *dest))
			{
				sendNumericAnswer(client, "404", name + " :Cannot send to channel");
				continue;
			}
			broadcast(*dest, msg, client.fd);
		}
		else
		{
			Client *dest_f = findClientByNick(name);
			if (!dest_f || !dest_f->registered)
			{
				sendNumericAnswer(client, "401", name + " :No such nick/channel");
				continue;
			}
			rawMessage(*dest_f, msg);
		}
	}
}
