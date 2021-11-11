//
// Created by Frey Tinkerer on 11/11/21.
//

#ifndef FT_IRC_COMMAND_HPP
#define FT_IRC_COMMAND_HPP

#include <string>
#include <vector>

namespace IRC {

class Command {
private:
	std::string					prefix;
	std::string					command;
	std::vector<std::string>	params;
	bool						valid;

public:
	Command(std::string const& message);
	virtual ~Command();

	const std::string &getPrefix() const;
	const std::string &getCommand() const;
	const std::vector<std::string> &getParams() const;
	bool isValid() const;
};

}

#endif //FT_IRC_COMMAND_HPP
