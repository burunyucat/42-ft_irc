#include "Server.hpp"

void Server::handlePass(Client& client, const Command& command)
{
	if (client.registered == true)
	{
		sendNumericAnswer(client, "462", ":You may not reregister");
		return;
	}
	if (command.params.empty())
	{
		sendNumericAnswer(client, "461", command.name + " :Not enough parameters");
		return;
	}
	if (command.params[0] == this->m_password)
		client.authenticated = true;
	else
		client.authenticated = false;
	tryCompleteRegistration(client);
}
