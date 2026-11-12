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
				makeActionDependingOfNbParameter(channel.key, command.params[2], client, command);
			else
				channel.key="";
		}
		else if(command.params[1][1] == 't')
			channel.topicRestricted = sign;
		else if(command.params[1][1] == 'o')
		{
			int a = 1;
			const int n = 1;
			int clientfd = -1;
			std::vector<int>::iterator membersIt;
			if(!makeActionDependingOfNbParameter(a, n, client, command))
				return ;
			for (std::map<int, Client>::iterator ClientIt = m_clientRegistry.begin(); ClientIt != m_clientRegistry.end(); ClientIt++)
			{
				if(ClientIt->second.nickname != command.params[2])
				{
					clientfd = ClientIt->second.fd;
					break;
				}
			}
			membersIt= std::find (channel.members.begin(), channel.members.end(), clientfd);
			if (clientfd != -1 && membersIt != channel.members.end())
			{
				std::vector<int>::iterator operatorIt = std::find(channel.operators.begin(), channel.operators.end(), clientfd);
				//Find Client
				if(sign == true)
				{
					if(operatorIt== channel.operators.end())
						channel.operators.push_back(clientfd);
				}
				else
				{
					if(operatorIt != channel.operators.end())
						channel.operators.erase(operatorIt); //Make reply if i erase operator of client
				}
			}
			else
			{
				// Not find Client
				if(clientfd == -1)
					Server::sendNumericAnswer(client, "401" , command.params[2] + " :Pas de nick/channel de ce genre");//ERR_NOSUCHNICK
				else
					Server::sendNumericAnswer(client, "441" ,command.params[2] +  " " + channel.name + " :They aren't on that channel");//ERR_USERNOTINCHANNEL
			}
		}
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
			Server::sendNumericAnswer(client, "472" , ":is unknown mode char to me for " + channel.name);//ERR_UNKNOWNMODE
	}
	else
		Server::sendNumericAnswer(client, "461" , ":Not enough parameters");//ERR_NEEDMOREPARAMS
}
