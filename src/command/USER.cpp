#include "Server.hpp"

static bool validUser(char c)
{
	if (c == 0 || c == 10 || c == 13 || c == 32 || c == 64)
		return false;
	return true;
}

void Server::handleUser(Client& client, const Command& command)
{
	if (client.registered == true)
	{
		sendNumericAnswer(client, "462", ":You may not reregister");
		return;
	}
	if (command.params.size() < 4)
	{
		sendNumericAnswer(client, "461", command.name + " :Not enough parameters");
		return;
	}
	std::string	test = command.params[0];
	if (test.length() < 1)
	{
		sendNumericAnswer(client, "461", command.name + " :Not enough parameters");
		return;
	}
	for (size_t i = 0; i < test.length() ; i++)
	{
		if (validUser(test[i]) == false)
		{
			/**
			 * Error 468 doesn't exist in the RFC 2812, but is used in many real server.
			 * We need to see if we keep it, or if we change the invalid character.
			 * The only invalid character that could pass is @, all other are caught by the parser or impossible to input.
			 */
			sendNumericAnswer(client, "468", command.name + " :invalid username");
			return ;
		}
	}
	client.username = command.params[0];
	client.realName = command.params[3];
	tryCompleteRegistration(client);
	return ;
}
