#include "Server.hpp"

static bool isSpecial(char c)
{
	return (c >= '[' && c <= '`') || (c >= '{' && c <= '}');
}

static bool nickValid(std::string nickname)
{
	if (nickname.length() < 1 || nickname.length() > 9)
		return false;
	if (!isalpha(nickname[0]) && !isSpecial(nickname[0]))
		return false;
	for (size_t i = 0; i < nickname.length(); i++)
		if (!isalnum(nickname[i]) && !isSpecial(nickname[i]))
			return false;
	return true;
}

void Server::handleNick(Client& client, const Command& command)
{
	if (command.params.empty())
	{
		sendNumericAnswer(client, "431", ":No nickname given");
		return;
	}
	if (nickValid(command.params[0]) == false)
	{
		sendNumericAnswer(client, "432", command.name + " :Erroneous nickname");
		return;
	}
	if (this->isNicknameInUse(command.params[0], client.fd) == true)
	{
		sendNumericAnswer(client, "433", command.name + " :Nickname is already in use");
		return;
	}
	if (client.registered == false)
		tryCompleteRegistration(client);
	/** @brief Second case, user is already registered and changes it's nickname.
	 * The following message needs to be sent:
	 * :ancien!user@localhost NICK nouveau
		else
	*/
}
