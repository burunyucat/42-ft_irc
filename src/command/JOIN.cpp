#include "Server.hpp"
#include "Channel.hpp"
// #include <iostream>

void Server::checkAddMembers(Channel &channel, const Command& command, Client& client)
{
	if(channel.memberLimit!=0 && (channel.members.size() + 1) >= channel.memberLimit)
		Server::sendNumericAnswer(client, "471" , channel.name + " :Cannot join channel (+l)");//ERR_CHANNELISFULL
	else if(channel.inviteOnly)
		Server::sendNumericAnswer(client, "473" , channel.name + " :Cannot join channel (+i)");//ERR_INVITEONLYCHAN
	else
		addMember(client.fd, channel);
}
static bool isPrefix(char prefix)
{
	if(prefix == '&' || prefix == '#' || prefix == '+' || prefix == '!')
		return true;
	else
		return false;
}
static bool parseNamesChannel(std::string channel)
{
	std::size_t found;
	found = channel.find('\a');
	if(channel.size()> 1 && channel.size()< 51 && isPrefix(channel[0]) && found == channel.npos)
		return true;
	else
		return false;
}
static void parseParamsJoin(const std::string &toExtract, std::vector<std::string> &str)
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
void Server::handleJoin(Client& client, const Command& command)
{
	std::vector<std::string> channel;
	Channel * ptrchannel;
	std::vector<std::string> keyOfChannel;
	std::vector<int>::iterator membersIt;
	if(command.params.size()>0)
	{
		if(command.params[0].size()==1 && command.params[0][0] == '0') //Option 0 quit all channel for the client
		{
			for (std::map<std::string, Channel>::iterator channelRegistryIt = m_channelRegistry.begin(); channelRegistryIt != m_channelRegistry.end(); channelRegistryIt++)
			{
				membersIt = std::find (channelRegistryIt->second.members.begin(), channelRegistryIt->second.members.end(), client.fd);
				if(membersIt !=channelRegistryIt->second.members.end())
					removeMember(client.fd, channelRegistryIt->second);
			}
		}
		else
		{
			parseParamsJoin(command.params[0], channel);//option 1 is one or multiple channels
			if(command.params.size()>1) //option 2 is one or multiple key of channels
				parseParamsJoin(command.params[1], keyOfChannel);
			for (size_t i = 0; i < channel.size(); i++)
			{
				ptrchannel = findChannel(channel[i]);
				if(ptrchannel)//Find channel
				{
					if(!ptrchannel->key.empty())
					{
						if(command.params.size()>1 && keyOfChannel.size()>=i && keyOfChannel[i] ==ptrchannel->key)
							checkAddMembers(*ptrchannel, command, client);
						else
							Server::sendNumericAnswer(client, "475" , channel[i] + " :Cannot join channel (+k)");//ERR_BADCHANNELKEY
					}
					else
						checkAddMembers(*ptrchannel, command, client);
				}
				else
				{
					if(parseNamesChannel(channel[i]))
					{//Create channel
						std::map<std::string, Channel>::iterator channelIt;
						channelIt = m_channelRegistry.lower_bound(channel[i]);
						m_channelRegistry.insert(channelIt, std::pair<std::string, Channel>(channel[i], Channel(channel[i])));
					}
					else
						Server::sendNumericAnswer(client, "476" ,  channel[i] + " :Bad Channel Mask");//ERR_BADCHANMASK
				}
			}
			
			//Test for parsing JOIN
			// for (size_t i = 0; i < channel.size(); i++)
			// 	std::cout << "channel "<< i <<channel[i] <<std::endl;
			// for (size_t i = 0; i < keyOfChannel.size(); i++)
			// 	std::cout << "keyOfChannel "<< i <<keyOfChannel[i] <<std::endl;
		}
	}
	else
		Server::sendNumericAnswer(client, "461" , ":Not enough parameters");//ERR_NEEDMOREPARAMS
}
