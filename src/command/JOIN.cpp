#include "Server.hpp"
#include "Channel.hpp"
// #include <iostream>


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
			if(command.params.size()>1) //option 2 is one or multiple key
				parseParamsJoin(command.params[1], keyOfChannel);
			for (size_t i = 0; i < channel.size(); i++)
			{
				ptrchannel = findChannel(channel[i]);
				if(ptrchannel)
				{
					if(!ptrchannel->key.empty())
					{
						if(command.params.size()>1 && keyOfChannel.size()>=i && keyOfChannel[i] ==ptrchannel->key)
							addMember(client.fd, *ptrchannel);
					}
					else
						addMember(client.fd, *ptrchannel);
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
