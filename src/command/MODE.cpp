#include "Server.hpp"
#include <cstdlib>
#include <sstream>

template <typename T>
static bool makeActionDependingOfNbParameter(T &leftVariable, const T &rightVariable, Client& client, const Command& command)
{
	if(command.params.size()>2)
	{
		leftVariable = rightVariable;
		return true;
	}
	else
	{
		Server::sendNumericAnswer(client, "461" , command.name + " :Not enough parameters"); //ERR_NEEDMOREPARAMS
		return false;
	}
}

void Server::handleModeOptionO(Client& client, const Command& command, Channel &channel, bool &sign)
{
	int a = 1;
	const int n = 1;
	Client *clientTarget;
	if(!makeActionDependingOfNbParameter(a, n, client, command))
		return ;
	clientTarget = findClientByNick(command.params[2]);
	if (clientTarget && isMember(clientTarget->fd, channel))
	{
		//Find Client
		if(sign == true)
		{
			addOperator(clientTarget->fd, channel);
			broadcast(channel, fullName(client) + " " + command.name + " " + channel.name + " " + command.params[1] + " " + command.params[2], -1);
		}
		else
		{
			removeOperator(clientTarget->fd, channel);
			broadcast(channel, fullName(client) + " " + command.name + " " + channel.name + " " + command.params[1] + " " + command.params[2], -1);
		}
	}
	else
	{
		// Not find Client
		if(!clientTarget)
			Server::sendNumericAnswer(client, "401" , command.params[2] + " :No such nick/channel");//ERR_NOSUCHNICK
		else
			Server::sendNumericAnswer(client, "441" ,command.params[2] +  " " + channel.name + " :They aren't on that channel");//ERR_USERNOTINCHANNEL
	}
}

void Server::handleMode(Client& client, const Command& command)
{
	bool sign;
	Channel *ptrChannel;
	if (command.params.size() ==  1)
	{
		if (!isPrefixChannel(command.params[0][0]))
			return ;
		ptrChannel = findChannel(command.params[0]);
		if(ptrChannel == NULL)
		{
			Server::sendNumericAnswer(client, "403" , command.params[0] + " :No such channel");//ERR_NOSUCHCHANNEL
			return ;
		}
		std::string modes = "+";
		std::string values = "";
		if (ptrChannel->inviteOnly == true)
			modes.append("i");
		if (ptrChannel->topicRestricted ==true)
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
		sendNumericAnswer(client, "324", ptrChannel->name + " " + modes + values);
		return ;
	}
	if(command.params.size() > 1 && command.params[1].size()==2 && (command.params[1][0] =='-' || command.params[1][0] =='+'))
	{
		if (!isPrefixChannel(command.params[0][0]))
			return ;
		ptrChannel = findChannel(command.params[0]);
		if(ptrChannel == NULL)
		{
			Server::sendNumericAnswer(client, "403" , command.params[0] + " :No such channel");//ERR_NOSUCHCHANNEL
			return ;
		}
		if(!isOperator(client.fd, *ptrChannel))
		{
			Server::sendNumericAnswer(client, "482" , ptrChannel->name + " :You're not channel operator");//ERR_CHANOPRIVSNEEDED
			return;
		}
		if(command.params[1][0] =='-')
			sign = false;
		else
			sign = true;
		//option Mode
		if(command.params[1][1] == 'i')
		{
			ptrChannel->inviteOnly =sign;
			broadcast(*ptrChannel, fullName(client) + " " + command.name + " " + ptrChannel->name + " " + command.params[1], -1);
		}
		else if(command.params[1][1] == 'k')
		{
			if(sign == true)
			{
				if (command.params.size() < 3)
					sendNumericAnswer(client, "461", command.name + " :Not enough parameters");
				else if (!ptrChannel->key.empty())
					Server::sendNumericAnswer(client, "467" , ptrChannel->name + " :Channel key already set");//ERR_KEYSET
				else
				{
					makeActionDependingOfNbParameter(ptrChannel->key, command.params[2], client, command);
					broadcast(*ptrChannel, fullName(client) + " " + command.name + " " + ptrChannel->name + " " + command.params[1] + " " + command.params[2], -1);
				}
			}
			else
			{
				ptrChannel->key="";
				broadcast(*ptrChannel, fullName(client) + " " + command.name + " " + ptrChannel->name + " " + command.params[1], -1);
			}
		}
		else if(command.params[1][1] == 't')
		{
			ptrChannel->topicRestricted = sign;
			broadcast(*ptrChannel, fullName(client) + " " + command.name + " " + ptrChannel->name + " " + command.params[1], -1);
		}
		else if(command.params[1][1] == 'o')
			Server::handleModeOptionO(client, command, *ptrChannel, sign);
		else if(command.params[1][1] == 'l')
		{
			if(sign == true)
			{
				if (command.params.size() < 3)
					Server::sendNumericAnswer(client, "461" , command.name + " :Not enough parameters");//ERR_KEYSET
				else
				{
					char *end;
					long convert =std::strtol(command.params[2].c_str(), &end, 10);
					if (end == command.params[2].c_str() || *end != '\0' || convert <= 0 )
						sendNumericAnswer (client, "696", ptrChannel->name + " l " + command.params[2] + " :Invalid mode parameter");
					else
					{
						makeActionDependingOfNbParameter(ptrChannel->memberLimit, static_cast<std::size_t>(convert), client, command);
						broadcast(*ptrChannel, fullName(client) + " " + command.name + " " + ptrChannel->name + " " + command.params[1] + " " + command.params[2], -1);
					}
				}
			}
			else
			{
				ptrChannel->memberLimit = 0;
				broadcast(*ptrChannel, fullName(client) + " " + command.name + " " + ptrChannel->name + " " + command.params[1], -1);
			}
		}
		else
			Server::sendNumericAnswer(client, "472" , std::string(1,command.params[1][1]) + " :is unknown mode char to me");//ERR_UNKNOWNMODE
	}
	else
		Server::sendNumericAnswer(client, "461" , command.name + " :Not enough parameters");//ERR_NEEDMOREPARAMS
}
