#pragma once

#include <string>
#include <map>
#include <vector>
#include <poll.h>
#include "Client.hpp"
#include "Channel.hpp"

class Server
{
	public:
		Server(int port, const std::string& password);

		// Main program
		void run();

	private:

		// Handle Command
		void handlePass();
		void handleNick();
		void handleUser();
		void handleJoin();
		void handlePrivmsg();
		void handleInvite();
		void handleMode();
		void handleKick();
		void handleTopic();
		void handlePart();
		void handleQuit();
		void handlePing();
		void handlePong();
		void handleNames();

		// Listening socket fd
		int m_listenFd;

		// Listening port
		int m_port;
		// Password to access the server
		std::string m_password;

		// Socket events monitored by poll
		std::vector<struct pollfd> m_pollFds;
		// Connected clients identified by FD
		std::map<int, Client> m_clients;

		// Channels identified by string keys
		std::map<std::string, Channel> m_channels;
};
