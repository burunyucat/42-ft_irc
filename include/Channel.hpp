/**
 * @file Channel.hpp
 * @brief Define IRC channel state, modes, and membership.
 */

#pragma once

#include <cstddef>
#include <string>
#include <vector>

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
