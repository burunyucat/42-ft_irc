#include "Server.hpp"

void Server::handleKick(Client& client, const Command& command)
{
	std::vector<std::string> channel;
	std::vector<std::string> user;
	Channel * ptrChannel;
	Client * targetClient;

	if(command.params.size() > 1)
	{
		splitComma(command.params[0], channel);
		splitComma(command.params[1], user);
	}
	if(command.params.size() > 1 && ((channel.size() == 1) || (channel.size()==user.size())))
	{
		for (size_t i = 0; i < user.size(); i++)
		{
			if(channel.size()!=1)
				ptrChannel = findChannel(channel[i]);
			else
				ptrChannel = findChannel(channel[0]);
			if(ptrChannel)
			{
				targetClient = findClientByNick(user[i]);
				if(!targetClient || !isMember(targetClient->fd, *ptrChannel))
					Server::sendNumericAnswer(client, "441" , user[i] + " " + ptrChannel->name + " :They aren't on that channel");//ERR_USERNOTINCHANNEL
				else if(!isMember(client.fd, *ptrChannel))
					Server::sendNumericAnswer(client, "442" , ptrChannel->name + " :You're not on that channel");//ERR_NOTONCHANNEL
				else if(!isOperator(client.fd, *ptrChannel))
					Server::sendNumericAnswer(client, "482" , ptrChannel->name + " :You're not channel operator");//ERR_CHANOPRIVSNEEDED
				else
				{
					if(command.params.size() > 2)//comment
						broadcast(*ptrChannel, fullName(client) +" KICK " + ptrChannel->name + " " +targetClient->nickname+ " :" + command.params[2], -1);
					else
						broadcast(*ptrChannel, fullName(client) +" KICK " + ptrChannel->name + " " +targetClient->nickname+ " :" + targetClient->nickname, -1);
					removeMember(targetClient->fd, *ptrChannel);
					if(ptrChannel->members.size() == 0)
					{
						std::map<std::string, Channel>::iterator it = this->m_channelRegistry.find(ptrChannel->name);
						m_channelRegistry.erase(it);
					}
				}
			}
			else
			{
				if(channel.size()!=1)
					Server::sendNumericAnswer(client, "403" , channel[i] + " :No such channel");//ERR_NOSUCHCHANNEL
				else
					Server::sendNumericAnswer(client, "403" , channel[0] + " :No such channel");//ERR_NOSUCHCHANNEL
			}
		}
	}
	else
		Server::sendNumericAnswer(client, "461" , command.name + " :Not enough parameters");//ERR_NEEDMOREPARAMS
}
