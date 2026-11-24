#include "Server.hpp"
#include <cstdlib>
#include <sstream>

// Append one applied mode letter to the summary, writing the sign only when it
// changes: "+i", "+t", "-k" gives "+it-k".
static void recordMode(std::string &applied, char &lastSign, bool sign, char mode)
{
	char currentSign = sign ? '+' : '-';
	if (lastSign != currentSign)
	{
		applied += currentSign;
		lastSign = currentSign;
	}
	applied += mode;
}

bool Server::handleModeOptionO(Client& client, Channel &channel, const std::string &nick, bool sign)
{
	Client *clientTarget = findClientByNick(nick);
	if (!clientTarget)
	{
		Server::sendNumericAnswer(client, "401", nick + " :No such nick/channel");//ERR_NOSUCHNICK
		return false;
	}
	if (!isMember(clientTarget->fd, channel))
	{
		Server::sendNumericAnswer(client, "441", nick + " " + channel.name + " :They aren't on that channel");//ERR_USERNOTINCHANNEL
		return false;
	}
	if (sign)
		addOperator(clientTarget->fd, channel);
	else
		removeOperator(clientTarget->fd, channel);
	return true;
}

void Server::handleMode(Client& client, const Command& command)
{
	Channel *ptrChannel;
	if (command.params.empty())
	{
		Server::sendNumericAnswer(client, "461", command.name + " :Not enough parameters");//ERR_NEEDMOREPARAMS
		return ;
	}
	// User modes (e.g. irssi sends "MODE nick +i" after connecting) are ignored.
	if (!isPrefixChannel(command.params[0][0]))
		return ;
	ptrChannel = findChannel(command.params[0]);
	if (ptrChannel == NULL)
	{
		Server::sendNumericAnswer(client, "403", command.params[0] + " :No such channel");//ERR_NOSUCHCHANNEL
		return ;
	}

	// "MODE #chan": show the current modes.
	if (command.params.size() == 1)
	{
		std::string modes = "+";
		std::string values = "";
		if (ptrChannel->inviteOnly == true)
			modes.append("i");
		if (ptrChannel->topicRestricted == true)
			modes.append("t");
		if (!ptrChannel->key.empty())
		{
			modes.append("k");
			values.append(" " + ptrChannel->key);
		}
		if (ptrChannel->memberLimit != 0)
		{
			modes.append("l");
			std::ostringstream oss;
			oss << ptrChannel->memberLimit;
			values.append(" " + oss.str());
		}
		sendNumericAnswer(client, "324", ptrChannel->name + " " + modes + values);//RPL_CHANNELMODEIS
		return ;
	}

	if (!isOperator(client.fd, *ptrChannel))
	{
		Server::sendNumericAnswer(client, "482", ptrChannel->name + " :You're not channel operator");//ERR_CHANOPRIVSNEEDED
		return ;
	}

	// "MODE #chan +kl-i secret 5": read params[1] letter by letter.
	// argIdx points to the next unused argument (k, l and o each take one).
	const std::string &modeString = command.params[1];
	bool sign = true;
	size_t argIdx = 2;
	std::string applied = "";
	std::string appliedArgs = "";
	char lastSign = 0;

	for (size_t i = 0; i < modeString.size(); i++)
	{
		char c = modeString[i];
		if (c == '+')
			sign = true;
		else if (c == '-')
			sign = false;
		else if (c == 'i')
		{
			ptrChannel->inviteOnly = sign;
			recordMode(applied, lastSign, sign, c);
		}
		else if (c == 't')
		{
			ptrChannel->topicRestricted = sign;
			recordMode(applied, lastSign, sign, c);
		}
		else if (c == 'k')
		{
			if (!sign)
			{
				// -k takes no argument here.
				ptrChannel->key = "";
				recordMode(applied, lastSign, sign, c);
			}
			else if (argIdx >= command.params.size())
				sendNumericAnswer(client, "461", command.name + " :Not enough parameters");//ERR_NEEDMOREPARAMS
			else
			{
				const std::string &key = command.params[argIdx++];
				if (!ptrChannel->key.empty())
					Server::sendNumericAnswer(client, "467", ptrChannel->name + " :Channel key already set");//ERR_KEYSET
				else
				{
					ptrChannel->key = key;
					recordMode(applied, lastSign, sign, c);
					appliedArgs += " " + key;
				}
			}
		}
		else if (c == 'l')
		{
			if (!sign)
			{
				ptrChannel->memberLimit = 0;
				recordMode(applied, lastSign, sign, c);
			}
			else if (argIdx >= command.params.size())
				sendNumericAnswer(client, "461", command.name + " :Not enough parameters");//ERR_NEEDMOREPARAMS
			else
			{
				const std::string &limit = command.params[argIdx++];
				char *end;
				long convert = std::strtol(limit.c_str(), &end, 10);
				if (end == limit.c_str() || *end != '\0' || convert <= 0)
					sendNumericAnswer(client, "696", ptrChannel->name + " l " + limit + " :Invalid mode parameter");//ERR_INVALIDMODEPARAM
				else
				{
					ptrChannel->memberLimit = static_cast<std::size_t>(convert);
					recordMode(applied, lastSign, sign, c);
					appliedArgs += " " + limit;
				}
			}
		}
		else if (c == 'o')
		{
			if (argIdx >= command.params.size())
				sendNumericAnswer(client, "461", command.name + " :Not enough parameters");//ERR_NEEDMOREPARAMS
			else
			{
				const std::string &nick = command.params[argIdx++];
				if (handleModeOptionO(client, *ptrChannel, nick, sign))
				{
					recordMode(applied, lastSign, sign, c);
					appliedArgs += " " + nick;
				}
			}
		}
		else
			Server::sendNumericAnswer(client, "472", std::string(1, c) + " :is unknown mode char to me");//ERR_UNKNOWNMODE
	}

	// One MODE line for everything that was actually applied.
	if (!applied.empty())
		broadcast(*ptrChannel, fullName(client) + " " + command.name + " " + ptrChannel->name + " " + applied + appliedArgs, -1);
}
