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
						notifyAllClientOnTheChannel(*ptrChannel, this->m_clientRegistry, client.nickname +" PART " +ptrChannel->name + " :" + command.params[1]);
					else
						notifyAllClientOnTheChannel(*ptrChannel, this->m_clientRegistry, client.nickname +" PART " +ptrChannel->name);
					removeMember(client.fd, *ptrChannel);
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
