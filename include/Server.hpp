#pragma once

#include <string>
#include "Channel.hpp"

class Server
{
	private:
		// Listening socket fd
		int m_listenFd;
		
		// Listening port
		int m_port;
		// Password to access the server
		std::string m_password;

		// Channels
		std::vector<Channel> m_channels;
		
		// Prevent server copu
		Server(const Server& other);
        Server& operator=(const Server& other);

	public:
		// Main program
		void run();

		// Handle JOIN command
		void handleJoin();
		// Handle MODE command
		void handleMode();
		// Handle KICK command
		void handleKick();
		// Handle TOPIC command
		void handleTopic();

		Server(int port, const std::string& password);
};

