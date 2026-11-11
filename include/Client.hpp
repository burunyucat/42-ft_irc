/**
 * @file Client.hpp
 * @brief Define client identity, registration and disconnection state, and buffers.
 */

#pragma once

#include <string>

struct Client
{
	// Connected socket fd
	int fd;

	// IRC nickname
	std::string nickname;
	// IRC username
	std::string username;
	// Real name provided by USER
	std::string realName;
	// Server password accepted
	bool authenticated;
	// IRC registration completed
	bool registered;

	// Request deferred disconnection after command execution.
	bool disconnectRequested;
	// Reason used in the IRC QUIT notification.
	std::string 	quitReason;

	// Received bytes awaiting processing.
	std::string inputBuffer;
	// Bytes awaiting transmission.
	std::string outputBuffer;

	// Default disconnected client
	Client() : fd(-1), authenticated(false), registered(false),
		disconnectRequested(false), quitReason("Connection closed") {}
};
