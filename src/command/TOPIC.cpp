#include "Server.hpp"

void Server::handleTopic(Client& client, const Command& command)
{
	Channel * ptrChannel;
	if(command.params.size()>0)
	{
		ptrChannel = findChannel(command.params[0]);
		if(!parseNamesChannel(command.params[0]))
			Server::sendNumericAnswer(client, "476" ,  command.params[0] + " :Bad Channel Mask");//ERR_BADCHANMASK
		else if(!ptrChannel)
			Server::sendNumericAnswer(client, "403" , command.params[0] + " :No such channel");//ERR_NOSUCHCHANNEL
		else if(!isMember(client.fd, *ptrChannel))
			Server::sendNumericAnswer(client, "442" , command.params[0] + " :You're not on that channel");//ERR_NOTONCHANNEL
		else
		{
			if(command.params.size()>1)
			{
				if(!ptrChannel->topicRestricted || isOperator(client.fd, *ptrChannel))
				{
					if(command.params[1].empty())//option for remove topic
						ptrChannel->topic="";
					else
						ptrChannel->topic=command.params[1];
				}
				else
					Server::sendNumericAnswer(client, "482" , ptrChannel->name + " :You're not channel operator");//ERR_CHANOPRIVSNEEDED
			}
			else//view Topic
			{
				if(ptrChannel->topic.empty())
					Server::sendNumericAnswer(client, "331", ptrChannel->name + " :No topic is set");//RPL_NOTOPIC
				else
					Server::sendNumericAnswer(client, "332", ptrChannel->name + " :"+ptrChannel->topic);//RPL_TOPIC
			}

		}
	}
	else
		Server::sendNumericAnswer(client, "461" , ":Not enough parameters");//ERR_NEEDMOREPARAMS
}
