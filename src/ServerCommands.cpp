/**
 * @file ServerCommands.cpp
 * @brief Parse IRC commands and dispatch them for execution.
 */

#include "Server.hpp"

bool Server::parseCommand(const std::string& line, Command& newCommand)
{
	(void)line;
	(void)newCommand;
	// TODO: parse the IRC line and populate newCommand.
	return true;
}

bool Server::executeCommand(Client& client, const Command& command)
{
	(void)client;
	(void)command;
	// TODO: dispatch the command to its handler.
	return true;
}
