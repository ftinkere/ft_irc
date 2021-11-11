//
// Created by Frey Tinkerer on 11/11/21.
//

#include <iostream>
#include "Command.hpp"

IRC::Command::Command(std::string const& message): valid(true) {
	if (message.empty() || message[0] == '\n' || message[0] == '\r') {
		valid = false;
		return;
	}

	std::string::size_type pos = 0;

	// Парсим prefix
	if (message[0] == ':') {
		std::string::size_type space_pos = message.find_first_of(' ');
		if (space_pos == std::string::npos) {
			valid = false;
			return;
		}
		prefix = message.substr(1, space_pos);
		pos = space_pos + 1;
	}

	{
		std::string::size_type command_end = message.find_first_of(' ', pos);

		if (command_end == std::string::npos) {
			if (message.find_first_of('\r') == std::string::npos) {
				command_end = message.find_first_of('\n', pos);
				if (command_end == std::string::npos) {
					valid = false;
					return;
				}
			} else {
				command_end = message.find_first_of('\n', pos) - 1;
				if (command_end == std::string::npos || message[command_end] != '\r') {
					valid = false;
					return;
				}
			}
		}
		command = message.substr(pos, command_end - pos);
		pos = command_end + 1;
	}

	std::cout << "[DEBUG]: prefix: \"" << prefix << "\", command: \"" << command << "\", valid: \"" << valid << "\"" << std::endl;

	{
		params = std::vector<std::string>();

		std::string::size_type space_pos;
		while ((space_pos = message.find_first_of(' ', pos)) != std::string::npos
				&& message[pos] != ':') {
			params.push_back(message.substr(pos, space_pos - pos));
			pos = space_pos + 1;
		}
		if (pos < message.length() && message[pos] != 'n' && message[pos] != '\r') {
			std::string::size_type offset = 0;
			if (message[message.length() - 1] == '\n')
				++offset;
			if (message.length() > 1 && message[message.length() - 2] == '\r')
				++offset;
			if (message[pos] == ':')
				++pos;
			params.push_back(message.substr(pos, message.length() - pos - offset));
		}
	}

	std::cout << "\t params: [";
	for (int i = 0; i < params.size() - 1; ++i) {
		std::cout << " " << params[i];
	}
	std::cout << " \"" << params[params.size() - 1] << "\" ]" << std::endl;
	std::cout << "[DEBUG]: original message: " << message << std::endl;
}

IRC::Command::~Command() {}

const std::string &IRC::Command::getPrefix() const { return prefix; }
const std::string &IRC::Command::getCommand() const { return command; }
const std::vector<std::string> &IRC::Command::getParams() const { return params; }
bool IRC::Command::isValid() const { return valid; }
