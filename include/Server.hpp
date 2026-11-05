/**
 * @file Server.hpp
 * @brief Declare the IRC server and its connection and command handling interface.
 */

#pragma once

#include <string>
#include <map>
#include <vector>
#include <poll.h>
#include "Client.hpp"
#include "Command.hpp"
#include "Channel.hpp"

class Server
{
	public:
		Server(int port, const std::string& password);
		~Server();

		/** @brief Run the server's main event loop. */
		void run();

	private:
		/** @brief Enable or disable client POLLOUT flags based on pending output. */
		void updateClientPolloutFlags();

		/**
		 * @brief Handle events reported for the listening socket.
		 * @param[in] revents Events reported by poll.
		 * @return true to keep running, false to stop the server.
		 */
		bool handleListenerEvents(short revents);

		/**
		 * @brief Handle receive, send, and error events for a client socket.
		 * @param[in] clientFd Client socket file descriptor.
		 * @param[in] revents Events reported by poll.
		 * @return true to keep the client, false to request deferred disconnection.
		 */
		bool handleClientEvents(int clientFd, short revents);

		/** @brief Accept and register a new nonblocking client connection. */
		void acceptClient();

		/**
		 * @brief Append received data and extract complete IRC lines.
		 * @details Stop processing commands when the client requests disconnection.
		 * @param[in,out] client Client whose input buffer is updated.
		 * @param[in] buffer Received data to append.
		 * @param[in] bytesRead Number of received bytes in buffer.
		 */
		void processClientInput(Client& client, const char* buffer, size_t bytesRead);

		/**
		 * @brief Send pending output and retain any unsent bytes.
		 * @param[in,out] client Client whose output buffer is updated.
		 * @return true if no data is pending or sending succeeds, false on failure.
		 */
		bool processClientOutput(Client& client);

		/**
		 * @brief Notify peers, close a client socket, and remove its records.
		 * @param[in] fd Socket file descriptor of the client to disconnect.
		 */
		void disconnectClient(int fd);

		/**
		 * @brief Queue an IRC QUIT notification for clients sharing a channel.
		 * @param[in] client Departing client, still registered in clients and channels.
		 * @details Append a CRLF-terminated message to each recipient's output buffer
		 * once, excluding the departing client. Do not send directly or remove records.
		 * Use client.quitReason as the notification reason.
		 */
		void notifyClientQuit(const Client& client);

		/**
		 * @brief Parse an IRC line into a command.
		 * @param[in] line IRC line without the trailing CRLF.
		 * @param[out] newCommand Command populated from the parsed line.
		 * @return true on success, false on failure.
		 */
		bool parseCommand(const std::string& line, Command& newCommand);

		/**
		 * @brief Execute the command sent by the client.
		 * @details For QUIT, set client.disconnectRequested and client.quitReason.
		 * Do not disconnect or notify directly; the TCP layer defers cleanup.
		 * @param[in,out] client Client issuing the command.
		 * @param[in] command Command to execute.
		 * @return true on success, false on a recoverable command failure.
		 */
		bool executeCommand(Client& client, const Command& command);

		/** @brief Handle the PASS command. */
		void handlePass();
		/** @brief Handle the NICK command. */
		void handleNick();
		/** @brief Handle the USER command. */
		void handleUser();
		/** @brief Handle the JOIN command. */
		void handleJoin();
		/** @brief Handle the PRIVMSG command. */
		void handlePrivmsg();
		/** @brief Handle the INVITE command. */
		void handleInvite();
		/** @brief Handle the MODE command. */
		void handleMode();
		/** @brief Handle the KICK command. */
		void handleKick();
		/** @brief Handle the TOPIC command. */
		void handleTopic();

		/** @brief Handle the PART command. */
		void handlePart();
		/** @brief Handle the QUIT command. */
		void handleQuit();
		/** @brief Handle the PING command. */
		void handlePing();
		/** @brief Handle the PONG command. */
		void handlePong();
		/** @brief Handle the NAMES command. */
		void handleNames();

		/** @brief Listening socket file descriptor. */
		int m_listenFd;

		/** @brief Listening port. */
		int m_port;
		/** @brief Password required to access the server. */
		std::string m_password;

		/** @brief Socket events monitored by poll. */
		std::vector<struct pollfd> m_pollFds;
		/** @brief Connected clients indexed by socket file descriptor. */
		std::map<int, Client> m_clientRegistry;

		/** @brief Channels indexed by string keys. */
		std::map<std::string, Channel> m_channelRegistry;
};
