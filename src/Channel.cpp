/**
 * @file Channel.cpp
 * @brief For all functions that directly impact the data stored in the Channel struct.
 */

#include "Channel.hpp"

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

void	addMember(int fd, Channel &channel)
{
	std::vector<int>::iterator it = std::find(channel.members.begin(), channel.members.end(), fd);
	if (it == channel.members.end())
		channel.members.push_back(fd);
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
