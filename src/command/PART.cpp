#include "Server.hpp"

void Server::handlePart(Client& client, const Command& command)
{
	std::vector<std::string> channel;
	Channel * ptrChannel;
	if(command.params.size()>0)
	{
		splitComma(command.params[0], channel);
		for (size_t i = 0; i < channel.size(); i++)
		{
			ptrChannel = findChannel(channel[i]);
			if(ptrChannel)
			{
				if(isMember(client.fd, channel[i]))
				{
					if(command.params.size()>1)
						broadcast(*ptrChannel, client.nickname +" PART " +ptrChannel->name + " :" + command.params[1], -1);
					else
						broadcast(*ptrChannel, client.nickname +" PART " +ptrChannel->name,-1);
					removeMember(client.fd, *ptrChannel);
					if(ptrChannel->members.size() == 0)
					{
						std::map<std::string, Channel>::iterator it = this->m_channelRegistry.find(ptrChannel->name);
						m_channelRegistry.erase(it);
					}
				}
				else
					Server::sendNumericAnswer(client, "442" , channel[i] + " :You're not on that channel");//ERR_NOTONCHANNEL
			}
			else
				Server::sendNumericAnswer(client, "403" , channel[i] + ":No such channel");//ERR_NOSUCHCHANNEL
		}
	}
	else
		Server::sendNumericAnswer(client, "461" , ":Not enough parameters");//ERR_NEEDMOREPARAMS
}
