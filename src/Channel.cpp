/**
 * @file Channel.cpp
 * @brief For all functions that directly impact the data stored in the Channel struct.
 */

#include "Channel.hpp"
#include "Server.hpp"

void splitComma(const std::string &toExtract, std::vector<std::string> &str)
{
	int nbOfComma;
	int index=0;
	size_t placeOfComma= 0;
	size_t oldPlaceOfComma =-1;
	nbOfComma = std::count (toExtract.begin(), toExtract.end(), ',');
	do
	{
		placeOfComma = toExtract.find(',', placeOfComma);
		str.push_back(toExtract.substr(oldPlaceOfComma+1, placeOfComma));
		index++;
		oldPlaceOfComma = placeOfComma;
	} while (index <= nbOfComma);
}


bool existChannel(std::string &name, std::map<std::string, Channel> &m_channelRegistry)
{
	std::map<std::string, Channel>::iterator channelRegistryIt;
	channelRegistryIt = std::find(m_channelRegistry.begin(), m_channelRegistry.end(), name);
	if(channelRegistryIt != m_channelRegistry.end())
		return true;
	else
		return false;
}
bool isMember(int fd, const Channel& channel)
{
	return std::find(channel.members.begin(), channel.members.end(), fd) != channel.members.end();
}

bool	isOperator(int fd, const Channel& channel)
{
	return std::find(channel.operators.begin(), channel.operators.end(), fd) != channel.operators.end();
}

bool	isInvited(int fd, const Channel& channel)
{
	return std::find(channel.invited.begin(), channel.invited.end(), fd) != channel.invited.end();
}

void	addMember(Client &client, Channel &channel, std::map<int, Client> &m_clientRegistry)
{
	std::vector<int>::iterator it = std::find(channel.members.begin(), channel.members.end(), client.fd);
	if (it == channel.members.end())
	{
		notifyAllClientOnTheChannel(channel, m_clientRegistry, client.nickname + " JOIN " + channel.name);
		std::string allNameOnChannel;
		channel.members.push_back(client.fd);
		if(!channel.topic.empty())
			Server::sendNumericAnswer(client, "332", channel.name + " :"+channel.topic);//RPL_TOPIC
		else
			Server::sendNumericAnswer(client, "331", channel.name + " :No topic is set");//RPL_NOTOPIC
		for (std::vector<int>::iterator it = channel.members.begin(); it != channel.members.end(); it++)
			allNameOnChannel+=*it+" ";
		Server::sendNumericAnswer(client, "353",  "= " + channel.name +" :"+ allNameOnChannel);//RPL_NAMREPLY
		Server::sendNumericAnswer(client, "366", channel.name + " :End of NAMES list");//RPL_ENDOFNAMES
	}
}

void	removeMember(int fd, Channel &channel)
{
	std::vector<int>::iterator it = std::find(channel.members.begin(), channel.members.end(), fd);
	if (it != channel.members.end())
		channel.members.erase(it);
	std::vector<int>::iterator it2 = std::find(channel.operators.begin(), channel.operators.end(), fd);
	if (it2 != channel.operators.end())
		channel.operators.erase(it2);
	std::vector<int>::iterator it3 = std::find(channel.invited.begin(), channel.invited.end(), fd);
	if (it3 != channel.invited.end())
		channel.invited.erase(it3);
}

void	addOperator(int fd, Channel &channel)
{
	std::vector<int>::iterator it = std::find(channel.operators.begin(), channel.operators.end(), fd);
	if (it == channel.operators.end())
		channel.operators.push_back(fd);
}

void	removeOperator(int fd, Channel &channel)
{
	std::vector<int>::iterator it = std::find(channel.operators.begin(), channel.operators.end(), fd);
	if (it != channel.operators.end())
		channel.operators.erase(it);
}

void	addInvited(int fd, Channel &channel)
{
	std::vector<int>::iterator it = std::find(channel.invited.begin(), channel.invited.end(), fd);
	if (it == channel.invited.end())
		channel.invited.push_back(fd);

}


void notifyAllClientOnTheChannel(Channel & channel, std::map<int, Client> &m_clientRegistry, const std::string & msg)
{
	std::map<int, Client>::iterator findIt;
	for (std::vector<int>::iterator it = channel.members.begin(); it != channel.members.end(); it++)
	{
		findIt = m_clientRegistry.find(*it);
		Server::sendNumericAnswer(findIt->second, "", msg);
	}
}