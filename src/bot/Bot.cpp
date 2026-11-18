#include "Bot.hpp"
#include <iostream>

Bot::Bot(const std::string &host, const std::string &port, const std::string &password) : m_host(host), m_port(port), m_password(password)
{
	this->m_fd = -1;
	this->m_nick = "Bot";
}
Bot::~Bot()
{
	if (this->m_fd != -1)
		close(this->m_fd);
}

/*
      toi                                  getaddrinfo
┌──────────────────┐                 ┌───────────────────────────────┐
│ hints (pile)     │ ── tes critères→│ cherche les adresses possibles│
│  ai_family  =    │                 │ ALLOUE une liste (malloc)     │
│     AF_INET      │                 └───────────────┬───────────────┘
│  ai_socktype =   │                                 │ remplit res
│     SOCK_STREAM  │                                 ▼
│  (le reste à 0)  │       res ──► ┌──────────────────────────┐
└──────────────────┘               │ ai_family / ai_socktype  │
                                   │ ai_protocol              │
  struct addrinfo *res;            │ ai_addr ──► sockaddr     │ ← pour connect
  (un simple pointeur,             │ ai_addrlen               │
   vide au départ)                 │ ai_next ──► (suivante)   │
                                   └──────────────────────────┘
*/

bool	Bot::connectToServer()
{
	struct addrinfo	hints;
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	struct addrinfo *res;
	if (getaddrinfo(m_host.c_str(), m_port.c_str(), &hints, &res) != 0)
		return false;
	m_fd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
	if (m_fd == -1)
	{
		freeaddrinfo(res);
		return (false);
	}
	if (connect(m_fd, res->ai_addr, res->ai_addrlen) == -1)
	{
		freeaddrinfo(res);
		close(m_fd);
		m_fd = -1;
		return false;
	}
	freeaddrinfo(res);
	return true;
}

bool	Bot::sendLine(const std::string& line)
{
	std::string out = line + "\r\n";
	size_t sent = 0;
	while(sent < out.size())
	{
		ssize_t check = send(m_fd, out.c_str() + sent, out.size() - sent, 0);
		if (check == -1)
			return false;
		else
			sent += check;
	}
	return true;
}

bool	Bot::parseLine(const std::string &line, std::string &prefix, Command &cmd)
{
	size_t	pos = line.find_first_not_of(" ", 0);
	if (pos == line.npos)
		return true;
	if (line[pos] == ':')
	{
		pos = line.find(" ", pos);
		prefix = line.substr(1, pos -1);
	}
	size_t	end = line.find(" ", pos);
	if (end != line.npos)
	{
		cmd.name = line.substr(pos, end - pos);
		for (size_t i = 0; i < cmd.name.size(); i++)
			cmd.name[i] = std::toupper(static_cast<unsigned char>(cmd.name[i]));
	}
	else
	{
		cmd.name = line.substr(pos);
		for (size_t i = 0; i < cmd.name.size(); i++)
			cmd.name[i] = std::toupper(static_cast<unsigned char>(cmd.name[i]));
		return true;
	}
	pos = end;
	while (pos != line.npos)
	{
		pos = line.find_first_not_of(" ", pos);
		if (pos == line.npos)
			break;
		if (line[pos] == ':')
		{
			cmd.params.push_back(line.substr(pos + 1));
			return true;
		}
		end = line.find(" ", pos);
		if (end != line.npos)
			cmd.params.push_back(line.substr(pos, end - pos));
		else
			cmd.params.push_back(line.substr(pos));
		pos = end;
	}
	return true;
}

bool	Bot::handleLine(const std::string &line)
{
	Command cmd;
	memset(&cmd, 0, sizeof(cmd));
	std::string	prefix;
	if (parseLine(line, prefix, cmd))
	{
		if (!cmd.name.empty())
			sendLine(cmd.name);
	}
}

void	Bot::run()
{
	if (!this->connectToServer())
	{
		std::cerr << "Bot failed to connect" << std::endl;
		return;
	}
	if (this->sendLine("PASS " + this->m_password) == false)
	{
		std::cerr << "Server error" << std::endl;
		return;
	}
	if (this->sendLine("NICK " + this->m_nick) == false)
	{
		std::cerr << "Server error" << std::endl;
		return;
	}
	if (this->sendLine("USER " + this->m_nick + " 0 * :IRC Bot") == false)
	{
		std::cerr << "Server error" << std::endl;
		return;
	}
	while (1)
	{
		char	buf[512];
		ssize_t	n = recv(this->m_fd, buf, sizeof(buf), 0);
		if (n == 0)
		{
			std::cerr << "Disconnected" << std::endl;
			return;
		}
		if (n < 0)
		{
			std::cerr << "Server error" << std::endl;
			return;
		}
		else
		{
			this->m_inputBuffer.append(buf, n);
			ssize_t pos = this->m_inputBuffer.find('\n');
			while (pos != std::string::npos)
			{
				std::string line = this->m_inputBuffer.substr(0, pos);
				if (!line.empty() && line[line.size() - 1] == '\r')
					line.erase(line.size() - 1, 1);
				// Remove the complete line and keep any incomplete data.
				this->m_inputBuffer.erase(0, pos + 1);
			}
		}
			std::cout.write(buf, n);
	}
}
