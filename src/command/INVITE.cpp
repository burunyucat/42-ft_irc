#include "Server.hpp"

void Server::handleInvite(Client& client, const Command& command)
{
	std::vector<std::string> user;
	if(command.params.size() == 2)
	{
		Client * targetClient;
		Channel * ptrChannel;
		user.push_back(command.params[0]);
		targetClient = findClientByNick(user[0]);
		ptrChannel = findChannel(command.params[1]);
		if(!parseNamesChannel(command.params[1]))
			Server::sendNumericAnswer(client, "476" ,  command.params[1] + " :Bad Channel Mask");//ERR_BADCHANMASK
		else if(!ptrChannel)
			Server::sendNumericAnswer(client, "403" , command.params[1] + " :No such channel");//ERR_NOSUCHCHANNEL
		else if(!targetClient)
			Server::sendNumericAnswer(client, "401" , command.params[0] + " :No such nick/channel");//ERR_NOSUCHNICK
		else if(isMember(targetClient->fd, *ptrChannel))
			Server::sendNumericAnswer(client, "443" , user[0] + " " + ptrChannel->name + " :is already on channel");//ERR_USERONCHANNEL
		else if(!isMember(client.fd, *ptrChannel))
			Server::sendNumericAnswer(client, "442" , ptrChannel->name + " :You're not on that channel");//ERR_NOTONCHANNEL
		else if(!isOperator(client.fd, *ptrChannel) && ptrChannel->inviteOnly)
			Server::sendNumericAnswer(client, "482" , ptrChannel->name + " :You're not channel operator");//ERR_CHANOPRIVSNEEDED
		else
		{
			rawMessage(*targetClient, fullName(client) + " INVITE " + targetClient->nickname + " "+ ptrChannel->name);
			sendNumericAnswer(client, "341", ptrChannel->name + " " + targetClient->nickname);
			addInvited(targetClient->fd, *ptrChannel);
		}
	}
	else
		Server::sendNumericAnswer(client, "461" , command.name + " :Not enough parameters");//ERR_NEEDMOREPARAMS
}
