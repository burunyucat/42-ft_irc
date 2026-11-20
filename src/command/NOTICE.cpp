#include "Server.hpp"

void Server::handleNotice(Client& client, const Command& command)
{
	if (command.params.empty() || command.params[0] == "")
		return;
	if (command.params.size() < 2 || command.params[1] == "")
		return;
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
			return;
		else if (!isMember(client.fd, *dest))
			return;
		broadcast(*dest, msg, client.fd);
	}
	else
	{
		Client *dest_f = findClientByNick(command.params[0]);
		if (!dest_f || !dest_f->registered)
			return;
		rawMessage(*dest_f, msg);
	}
}
