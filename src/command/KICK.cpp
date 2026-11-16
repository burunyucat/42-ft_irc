#include "Server.hpp"

void Server::handleKick(Client& client, const Command& command)
{
	std::vector<std::string> channel;
	std::vector<std::string> user;
	Channel * ptrChannel;

	if(command.params.size() > 1)
	{
		splitComma(command.params[0], channel);
		splitComma(command.params[1], user);
	}
	if(command.params.size() > 1 && ((channel.size() == 1) || (channel.size()==user.size())))
	{
		// if(command.params.size() > 2)//comment
			
		
		for (size_t i = 0; i < channel.size(); i++)
		{
			ptrChannel = findChannel(channel[i]);
			if(ptrChannel)
			{
				if(channel.size()==1)
					for (size_t i = 0; i < user.size(); i++)
					{
						/* code */
					}
				if()//create a function on channel.cpp for kick and mode he make user exist on channel
					Server::sendNumericAnswer(client, "441" , user[i] + " " + channel[i] + " :They aren't on that channel");//ERR_USERNOTINCHANNEL
				else if(!isMember(client.fd, channel[i]))
					Server::sendNumericAnswer(client, "442" , channel[i] + " :You're not on that channel");//ERR_NOTONCHANNEL
				else if(!isOperator(client.fd, channel[i]))
					Server::sendNumericAnswer(client, "482" , channel[i] + " :You're not channel operator");//ERR_CHANOPRIVSNEEDED
				else
				{//to modify
					if(command.params.size()>1)
						notifyAllClientOnTheChannel(*ptrChannel, this->m_clientRegistry, "PART " +ptrChannel->name + " :" + command.params[1]);
					else
						notifyAllClientOnTheChannel(*ptrChannel, this->m_clientRegistry, "PART " +ptrChannel->name);
					removeMember(client.fd, *ptrChannel);
				}
			}
			else
				Server::sendNumericAnswer(client, "403" , channel[i] + ":No such channel");//ERR_NOSUCHCHANNEL
		}
		
	}
	else
		Server::sendNumericAnswer(client, "461" , ":Not enough parameters");//ERR_NEEDMOREPARAMS
}
