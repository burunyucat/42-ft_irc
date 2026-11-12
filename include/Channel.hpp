/**
 * @file Channel.hpp
 * @brief Define IRC channel state, modes, and membership.
 */

#pragma once

#include <cstddef>
#include <string>
#include <vector>
#include <algorithm>
#include <map>

struct Channel
{
	// Channel name
	std::string name;

	// Access on invitation only (+i)
	bool inviteOnly;
	// Topic changes restricted to operators (+t)
	bool topicRestricted;
	// Entry key (+k), empty when disabled
	std::string key;
	// Member limit (+l), zero when disabled
	std::size_t memberLimit;

	// Member client FDs
	std::vector<int> members;
	// Operator client FDs (+o)
	std::vector<int> operators;
	// Invited client FDs
	std::vector<int> invited;
	std::string topic;

	// Default unrestricted channel
	Channel() : inviteOnly(false), topicRestricted(false), memberLimit(0) {}
};

	/**
	 * Functions for consulting the data in the Channel struct.
	 */
	bool existChannel(std::string &name, std::map<std::string, Channel> &m_channelRegistry);
	bool	isMember(int fd, const Channel& channel);
	bool	isOperator(int fd, const Channel& channel);
	bool	isInvited(int fd, const Channel& channel);

	/**
	 * Functions for changing the data in the Channel struct.
	 */
	void	addMember(int fd, Channel &channel);
	/* Removes the fd of all 3 vectors*/
	void	removeMember(int fd, Channel &channel);
	void	addOperator(int fd, Channel &channel);
	/* Only remove the operator vector*/
	void	removeOperator(int fd, Channel &channel);
	void	addInvited(int fd, Channel &channel);
