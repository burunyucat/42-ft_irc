/**
 * @file ServerUtils.cpp
 * @brief Server helpers: client and channel lookup, message delivery, and JOIN notifications.
 */

#include "Server.hpp"
#include "Channel.hpp"
#include <cctype>
#include <cstddef>
#include <map>
#include <string>
#include <vector>

bool Server::isNicknameInUse(const std::string &nick, int exceptFd)
{
	std::string	tested = nick;
	for (size_t j = 0; j < nick.length(); j++)
		tested[j] = tolower(nick[j]);
	for (std::map<int, Client>::iterator it = this->m_clientRegistry.begin(); it != this->m_clientRegistry.end(); ++it)
	{
		// A departed client no longer holds its nickname.
		if (it->first == exceptFd || it->second.departed)
			continue;
		std::string registered = it->second.nickname;
		for (size_t i = 0; i < it->second.nickname.length(); i++)
			registered[i] = tolower(it->second.nickname[i]);
		if (tested == registered)
			return true;
	}
	return false;
}

Client *Server::findClientByNick(const std::string &nick)
{
	std::string	tested = nick;
	for (size_t j = 0; j < nick.length(); j++)
		tested[j] = tolower(nick[j]);
	for (std::map<int, Client>::iterator it = this->m_clientRegistry.begin(); it != this->m_clientRegistry.end(); ++it)
	{
		// A departed client is no longer reachable by nickname.
		if (it->second.departed)
			continue;
		std::string registered = it->second.nickname;
		for (size_t i = 0; i < it->second.nickname.length(); i++)
			registered[i] = tolower(it->second.nickname[i]);
		if (tested == registered)
			return &it->second;
	}
	return NULL;
}

void Server::rawMessage(Client &client, const std::string &message)
{
	if (message.empty())
		return;
	std::string out = message + "\r\n";
	client.outputBuffer.append(out);
}

std::string Server::fullName(const Client &client) const
{
	std::string out = ":" + client.nickname + "!" + client.username + "@localhost";
	return out;
}

void Server::broadcast(const Channel &channel, const std::string &message, int exceptFd)
{
	for (size_t i = 0; i < channel.members.size(); i++)
	{
		if (channel.members[i] == exceptFd)
			continue;
		std::map<int, Client>::iterator it = this->m_clientRegistry.find(channel.members[i]);
		if (it == this->m_clientRegistry.end())
			continue;
		this->rawMessage(it->second, message);
	}
}

Channel *Server::findChannel(const std::string &name)
{
	std::map<std::string, Channel>::iterator it = this->m_channelRegistry.find(name);
	if (it == m_channelRegistry.end())
		return NULL;
	return &it->second;
}

void Server::addMemberToChannelNotification(Channel &channel,Client &client)
{
	broadcast(channel, fullName(client) + " JOIN " + channel.name, -1);
	if(!channel.topic.empty())
		Server::sendNumericAnswer(client, "332", channel.name + " :"+channel.topic);//RPL_TOPIC
	else
		Server::sendNumericAnswer(client, "331", channel.name + " :No topic is set");//RPL_NOTOPIC
	sendNamesReply(client, channel);
	Server::sendNumericAnswer(client, "366", channel.name + " :End of NAMES list");//RPL_ENDOFNAMES
}
