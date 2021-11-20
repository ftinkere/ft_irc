//
// Created by Frey Tinkerer on 11/11/21.
//

#ifndef FT_IRC_COMMAND_HPP
#define FT_IRC_COMMAND_HPP

#include <string>
#include <vector>

class Client;

namespace IRC {

class ListenSocket;

class Command {
private:
	std::string					prefix;
	std::string					command;
	std::vector<std::string>	params;
	bool						valid;

public:
	Command(std::string const& message);
	Command(const std::string &prefix, const std::string &command, const std::vector<std::string> &params);

	virtual ~Command();

	const std::string &getPrefix() const;
	const std::string &getCommand() const;
	const std::vector<std::string> &getParams() const;
	bool isValid() const;

	void exec(Client & client, ListenSocket & server) const;
	void send_to(Client const& client, ListenSocket const& server) const;
	void send_to(std::string const& nick, ListenSocket const& server) const;
};

}

#endif //FT_IRC_COMMAND_HPP
