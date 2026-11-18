/**
 * @file Bot.hpp
 * @brief Define the name and parameters of the bot.
 */

#pragma once

#include <string>
#include <netdb.h>
#include <sys/socket.h>
#include <unistd.h>
#include <cstring>
#include "Command.hpp"
#include "Server.hpp"

class Bot
{
	private:
	std::string	m_host;
	std::string	m_port;
	std::string	m_password;
	std::string	m_nick;
	std::string	m_inputBuffer;
	int	m_fd;

	bool	connectToServer();
	bool	sendLine(const std::string& line);
	bool	parseLine(const std::string &line, std::string &prefix, Command &cmd);
	bool	handleLine(const std::string &line);

	Bot	&operator=(const Bot &other);
	Bot(const Bot &other);

	public:
	Bot(const std::string &host, const std::string &port, const std::string &password);
	~Bot();
	void	run();
};
