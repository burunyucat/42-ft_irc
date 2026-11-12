#include "Server.hpp"

static bool isSpecial(char c)
{
	return (c >= '[' && c <= '`') || (c >= '{' && c <= '}');
}

static bool nickValid(const std::string& nickname)
{
	if (nickname.length() < 1 || nickname.length() > 9)
		return false;
	if (!isalpha(static_cast<unsigned char>(nickname[0])) && !isSpecial(nickname[0]))
		return false;
	for (size_t i = 0; i < nickname.length(); i++)
		if (!isalnum(static_cast<unsigned char>(nickname[i])) && !isSpecial(nickname[i]))
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
		sendNumericAnswer(client, "432", command.params[0] + " :Erroneous nickname");
		return;
	}
	if (this->isNicknameInUse(command.params[0], client.fd) == true)
	{
		sendNumericAnswer(client, "433", command.params[0] + " :Nickname is already in use");
		return;
	}
	if (client.registered == false)
	{
		client.nickname = command.params[0];
		tryCompleteRegistration(client);
	}
	else
	{
		if (client.nickname == command.params[0])
			return;
		std::string out = fullName(client) + " NICK " + command.params[0];
		client.nickname = command.params[0];
		rawMessage(client, out);
	}
}
