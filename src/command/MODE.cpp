#include "Server.hpp"

template <typename T>
bool makeActionDependingOfNbParameter(T &leftVariable, const T &rightVariable, Client& client, const Command& command)
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
void Server::handleMode(Client& client, const Command& command)
{
	bool sign;
	Channel channel;
	std::map<std::string, Channel>::iterator channelRegistryIt;
	(void)client;
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
				Server::makeActionDependingOfNbParameter(channel.key, command.params[2], client, command);
			else
				channel.key="";
		}
		else if(command.params[1][1] == 't')
			channel.topicRestricted = sign;
		else if(command.params[1][1] == 'o')
		{
			const int n =1;
			if(!Server::makeActionDependingOfNbParameter(1, n, client, command))
				return ;
			for (std::map<int, Client>::iterator ClientIt = m_clientRegistry.begin(); ClientIt->second.nickname != command.params[2]; ClientIt++)
			{
				if
			}
			
			std::vector<int>::iterator membersIt = std::find(channel.members.begin(), channel.members.end(), command.params[2]);
			std::vector<int>::iterator operatorIt;
			if (membersIt != channel.members.end())// && etc
			{
				//Find
				if(sign == true)
				{

					channel.operators.push_back(*membersIt);
				}
				else
				{
					channel.operators.erase(membersIt);
				}
			}
			else
			{
				// Not find
			}
		}
		else if(command.params[1][1] == 'l')
		{
			if(sign == true)
			{
				int convert =std::atoi(command.params[2].c_str());
				Server::makeActionDependingOfNbParameter(channel.memberLimit, static_cast<std::size_t>(convert), client, command);
			}
			else
				channel.memberLimit = 0;
		}
		else
			Server::sendNumericAnswer(client, "472" , ":is unknown mode char to me for " + channel.name);//ERR_UNKNOWNMODE
	}
	else
		Server::sendNumericAnswer(client, "461" , ":Not enough parameters");//ERR_NEEDMOREPARAMS
}
