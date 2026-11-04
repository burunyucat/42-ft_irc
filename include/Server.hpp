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
		~Server();

		// Main program
		void run();

	private:
		// Accept and register a new nonblocking client connection.
		void acceptClient();

		// Append received data to the client's input buffer and extract complete IRC lines.
		void processClientInput(Client& client, const char* buffer, size_t bytesRead);

		// Close a client socket and remove its server and channel records.
		void disconnectClient(int fd);

		// Handle PASS command
		void handlePass();
		// Handle NICK command
		void handleNick();
		// Handle USER command
		void handleUser();
		// Handle JOIN command
		void handleJoin();
		// Handle PRIVMSG command
		void handlePrivmsg();
		// Handle INVITE command
		void handleInvite();
		// Handle MODE command
		void handleMode();
		// Handle KICK command
		void handleKick();
		// Handle TOPIC command
		void handleTopic();

		// Handle PART command
		void handlePart();
		// Handle QUIT command
		void handleQuit();
		// Handle PING command
		void handlePing();
		// Handle PONG command
		void handlePong();
		// Handle NAMES command
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
