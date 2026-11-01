#pragma once

#include "Client.hpp"
#include <vector>

struct Channel
{
	bool inviteOnly;

	std::vector<Client> members;
	std::vector<Client> operators;
	std::string topic;

	Channel() : inviteOnly(false) {}
};
