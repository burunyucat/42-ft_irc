#include "Server.hpp"

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
		Server::sendNumericAnswer(client, "461" , ":Not enough parameters"); //ERR_NEEDMOREPARAMS
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
			addOperator(clientTarget->fd, channel);
		else
			removeOperator(clientTarget->fd, channel);
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
	Channel channel;
	std::map<std::string, Channel>::iterator channelRegistryIt;
	if(command.params.size() > 1 && command.params[1].size()==2 && (command.params[1][0] =='-' || command.params[1][0] =='+'))
	{
		channelRegistryIt = m_channelRegistry.find(command.params[0]);
		if(channelRegistryIt == m_channelRegistry.end())
		{
			Server::sendNumericAnswer(client, "403" , command.params[0] + ":No such channel");//ERR_NOSUCHCHANNEL
			return ;
		}
		channel = m_channelRegistry[command.params[0]];
		if(command.params[1][0] =='-')
			sign = false;
		else
			sign = true;
		//option Mode
		if(command.params[1][1] == 'i')
			channel.inviteOnly =sign;
		else if(command.params[1][1] == 'k')
		{
			if(sign == true)
			{
				if(channel.key.empty())
					makeActionDependingOfNbParameter(channel.key, command.params[2], client, command);
				else
					Server::sendNumericAnswer(client, "467" , channel.name + " :Channel key already set");//ERR_KEYSET
			}
			else
				channel.key="";
		}
		else if(command.params[1][1] == 't')
			channel.topicRestricted = sign;
		else if(command.params[1][1] == 'o')
			Server::handleModeOptionO(client, command, channel, sign);
		else if(command.params[1][1] == 'l')
		{
			if(sign == true)
			{
				int convert =std::atoi(command.params[2].c_str());
				makeActionDependingOfNbParameter(channel.memberLimit, static_cast<std::size_t>(convert), client, command);
			}
			else
				channel.memberLimit = 0;
		}
		else
			Server::sendNumericAnswer(client, "472" , command.params[0] + " :is unknown mode char to me for " + channel.name);//ERR_UNKNOWNMODE
	}
	else
		Server::sendNumericAnswer(client, "461" , ":Not enough parameters");//ERR_NEEDMOREPARAMS
}
