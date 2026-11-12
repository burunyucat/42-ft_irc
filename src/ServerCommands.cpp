/**
 * @file ServerCommands.cpp
 * @brief Parse and execute IRC commands and prepare quit notifications.
 */

#include "Server.hpp"
#include <cctype>

bool Server::parseCommand(const std::string& line, Command& newCommand)
{
	size_t	pos = line.find_first_not_of(" ", 0);
	if (pos == line.npos)
		return true;
	if (line[pos] == ':')
	{
		pos = line.find(" ", pos);
		if (pos == line.npos)
			return false;
		pos = line.find_first_not_of(" ", pos);
		if (pos == line.npos)
			return false;
	}
	size_t	end = line.find(" ", pos);
	if (end != line.npos)
	{
		newCommand.name = line.substr(pos, end - pos);
		for (size_t i = 0; i < newCommand.name.size(); i++)
			newCommand.name[i] = std::toupper(static_cast<unsigned char>(newCommand.name[i]));
	}
	else
	{
		newCommand.name = line.substr(pos);
		for (size_t i = 0; i < newCommand.name.size(); i++)
			newCommand.name[i] = std::toupper(static_cast<unsigned char>(newCommand.name[i]));
		return true;
	}
	pos = end;
	while (pos != line.npos)
	{
		pos = line.find_first_not_of(" ", pos);
		if (pos == line.npos)
			break;
		if (line[pos] == ':')
		{
			newCommand.params.push_back(line.substr(pos + 1));
			return true;
		}
		end = line.find(" ", pos);
		if (end != line.npos)
			newCommand.params.push_back(line.substr(pos, end - pos));
		else
			newCommand.params.push_back(line.substr(pos));
		pos = end;
	}
	return true;
}

void Server::sendNumericAnswer(Client &client, const std::string &num, const std::string &message)
{
	std::string out = ":ircserv " + num + " ";
	if (!client.nickname.empty())
		out = out + client.nickname + " ";
	else
		out = out + "*" + " ";
	out += message + "\r\n";
	client.outputBuffer.append(out);
}

void Server::tryCompleteRegistration(Client& client)
{
	if (client.registered == true)
		return;
	else if (!client.nickname.empty() && !client.username.empty() && client.authenticated == false)
	{
		this->sendNumericAnswer(client, "464", ":Password incorrect");
		client.quitReason = "Bad password";
		client.disconnectRequested = true;
		return;
	}
	else if (client.nickname.empty() || client.username.empty())
		return;
	client.registered = true;
	/* For the 001, instead of localhost, we may change that by tracking the current host through the client structure*/
	this->sendNumericAnswer(client, "001", ":Welcome to the Internet Relay Network " + client.nickname + "!" + client.username + "@localhost");
	this->sendNumericAnswer(client, "002", ":Your host is ircserv, running version 1.0");
	this->sendNumericAnswer(client, "003", ":This server was created " + static_cast<std::string>(__DATE__));
	this->sendNumericAnswer(client, "004", "ircserv 1.0 o itkol");
	/* Option B for the execution, we check idependently and send a specialized return on the cause of rejection.
	else
	{
		this->sendNumericAnswer(client, "464", ":Password incorrect");
	}
	*/
}

void Server::executeCommand(Client& client, const Command& command)
{
	if (command.name.empty())
		return ;
	std::map<std::string, Handler>::iterator it = this->m_commandHandler.find(command.name);
	if (it == this->m_commandHandler.end())
		this->sendNumericAnswer(client, "421", command.name + " :Unknown command");
	else if (client.registered == false && !this->m_authorizedHandler.count(command.name))
		this->sendNumericAnswer(client, "451", ":You have not registered");
	else
		(this->*it->second)(client, command);
}

void Server::handleQuit(Client& client, const Command& command)
{
	if (!command.params.empty())
		client.quitReason = command.params[0];
	client.disconnectRequested = true;
}

void Server::notifyClientQuit(const Client& client)
{
	(void)client;
	// TODO: queue QUIT once per shared-channel peer, excluding the departing client.
	// Use client.quitReason; client identity and channel memberships are still available.
}
