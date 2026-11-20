#include "Server.hpp"
#include "Channel.hpp"

void Server::sendNamesReply(Client& client, const Channel& channel)
{
	// Size of ":ircserv 353 <nick> = <channel> :" + "\r\n", so each line fits in 512 bytes.
	const std::string nick = client.nickname.empty() ? "*" : client.nickname;
	const size_t overhead = std::string(":ircserv 353 ").size() + nick.size()
		+ std::string(" = ").size() + channel.name.size() + std::string(" :").size();
	const size_t budget = (LIMIT_LINE > overhead) ? LIMIT_LINE - overhead : 0;

	std::string names;
	for (size_t i = 0; i < channel.members.size(); i++)
	{
		std::map<int, Client>::iterator it = m_clientRegistry.find(channel.members[i]);
		if (it == m_clientRegistry.end())
			continue;
		std::string entry = (isOperator(it->second.fd, channel) ? "@" : "") + it->second.nickname;
		// Start a new RPL_NAMREPLY line when this entry would exceed the line limit.
		if (!names.empty() && names.size() + 1 + entry.size() > budget)
		{
			sendNumericAnswer(client, "353", "= " + channel.name + " :" + names);//RPL_NAMREPLY
			names.clear();
		}
		if (!names.empty())
			names.append(" ");
		names.append(entry);
	}
	if (!names.empty())
		sendNumericAnswer(client, "353", "= " + channel.name + " :" + names);//RPL_NAMREPLY
}

void Server::handleNames(Client& client, const Command& command)
{
	// No parameter: list every channel, then a single end of list.
	if (command.params.empty() || command.params[0].empty())
	{
		for (std::map<std::string, Channel>::iterator it = m_channelRegistry.begin(); it != m_channelRegistry.end(); ++it)
			sendNamesReply(client, it->second);
		sendNumericAnswer(client, "366", "* :End of NAMES list");//RPL_ENDOFNAMES
		return;
	}

	// One or more channels: an unknown channel only gets the end of list (RFC 2812).
	std::vector<std::string> channels;
	splitComma(command.params[0], channels);
	for (size_t i = 0; i < channels.size(); i++)
	{
		Channel *channel = findChannel(channels[i]);
		if (channel)
			sendNamesReply(client, *channel);
		sendNumericAnswer(client, "366", channels[i] + " :End of NAMES list");//RPL_ENDOFNAMES
	}
}
