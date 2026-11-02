#include "Server.hpp"


void Server::handleRedirectionCommand()
{
	if(command.name == "PASS")
		Server::handlePass();
	else if(command.name == "INVITE")
		Server::handleInvite();
	else if(command.name == "JOIN")
		Server::handleJoin();
	else if(command.name == "KICK")
		Server::handleKick();
	else if(command.name == "MODE")
		Server::handleMode();
	else if(command.name == "NICK")
		Server::handleNick();
	else if(command.name == "PASS")
		Server::handlePass();
	else if(command.name == "PRIVMSG")
		Server::handlePrivmsg();
	else if(command.name == "TOPIC")
		Server::handleTopic();
	else if(command.name == "USER")
		Server::handleUser();
	else
		std::cerr << command.name <<"Unknown command"<<std::endl;
}

int Server::parsingCommand(std::string line)
{
	Commands command;
	int words;
	int indexstart;

	words = 0;
	indexstart = 0;
	for (size_t i = 0; i <= line.size(); i++)
	{
		if(i == line.size() || line[i] == ' ')
		{
			if(words == 0)
				command.name = line.substr(indexstart, i);
			else if(words > 0)
			{
				if(line[indexstart] == ':')
				{
					command.params.push_back(line.substr(indexstart +1, line.size() -indexstart-1));
					break;
				}
				else
					command.params.push_back(line.substr(indexstart, i - indexstart));
			}
			words ++;
			while(line[i] == ' ' && i < line.size())
				i++;
			if(line[i] != ' ' && i < line.size())
				indexstart = i;
		}
	}
	// Test parsing
	// std::cout << "name = " << command.name << ";" <<std::endl;
	// for (size_t i = 0; i < command.params.size(); i++)
		// std::cout << "params["<< i <<"] = " << command.params[i] << ";"<<std::endl;
	if(words <= 1)
	{
		std::cerr << "Insufficient arguments for command."<<std::endl;
		return 1;
	}
	else
		return 0;
}
