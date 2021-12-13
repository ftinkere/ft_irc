//
// Created by Frey Tinkerer on 11/11/21.
//

#include "Command.hpp"
#include <Parser.hpp>
#include <Reply.hpp>
#include <algorithm>
#include <iostream>

namespace IRC {
	char to_upper(char &c) {
		c = (char) std::toupper((int) c);
		return c;
	}

IRC::Command::Command(std::string const& message): valid(true) {
	if (message.empty() || message[0] == '\n' || message[0] == '\r') {
		valid = false;
		return;
	}

		std::string::size_type pos = 0;

		while (message[pos] == ' ') {
			++pos;
		}

		// Парсим prefix
		if (message[pos] == ':') {
			std::string::size_type space_pos = message.find_first_of(' ', pos);
			if (space_pos == std::string::npos) {
				valid = false;
				return;
			}
			prefix = message.substr(pos + 1, space_pos - pos - 1);
			pos = space_pos + 1;
		}

		{
			while (message[pos] == ' ') {
				++pos;
			}
			std::string::size_type command_end = message.find_first_of(' ', pos);

			if (command_end == std::string::npos) {
				command_end = message.size();
			}
			command = message.substr(pos, command_end - pos);
			std::string buf;
			std::transform(command.begin(), command.end(), std::back_inserter(buf), &to_upper);
			pos = command_end + 1;
		}

		std::cout << "[DEBUG]: prefix: \"" << prefix << "\", command: \"" << command << "\", valid: \"" << valid << "\""
				  << std::endl;

		{
			params = std::vector<std::string>();

			std::string::size_type space_pos;
			while ((space_pos = message.find_first_of(' ', pos)) != std::string::npos
				   && message[pos] != ':') {
				if (space_pos != pos) {
					params.push_back(message.substr(pos, space_pos - pos));
				}
				pos = space_pos + 1;
			}
			if (pos < message.length() && message[pos] != '\n') {
				std::string::size_type offset = 0;
				if (message[message.length() - 1] == '\n')
					++offset;
				if (message[pos] == ':')
					++pos;
				params.push_back(message.substr(pos, message.length() - pos - offset));
			}
		}

		std::cout << "\t params: [";
		for (int i = 0; !params.empty() && i < params.size() - 1; ++i) {
			std::cout << " " << params[i];
		}
		if (!params.empty()) {
			std::cout << " \"" << params[params.size() - 1] << "\"";
		}
		std::cout << " ]" << std::endl;
	//	std::cout << "[DEBUG]: original message: " << message << std::endl;
	}

	IRC::Command::Command(const std::string &prefix, const std::string &command, const std::vector<std::string> &params)
			: prefix(prefix), command(command), params(params), valid(true) {}
//	IRC::Command::Command(const std::string &prefix, const std::string &command)
//			: prefix(prefix), command(command), params(std::vector<std::string>()), valid(true) {}

	Command::Command(const std::string &prefix, const std::string &command, const std::string &arg1,
					 const std::string &arg2, const std::string &arg3, const std::string &arg4)
					 : prefix(prefix), command(command), params(std::vector<std::string>()), valid(true) {
		if (!arg1.empty())
			params.push_back(arg1);
		if (!arg2.empty())
			params.push_back(arg2);
		if (!arg3.empty())
			params.push_back(arg3);
		if (!arg4.empty())
			params.push_back(arg4);
	}


	IRC::Command::~Command() {}

	const std::string &IRC::Command::getPrefix() const { return prefix; }
	const std::string &IRC::Command::getCommand() const { return command; }
	const std::vector<std::string> &IRC::Command::getParams() const { return params; }
	std::vector<std::string> &IRC::Command::getParams() { return params; }
	bool IRC::Command::isValid() const { return valid; }

	std::string Command::to_string() const {
		std::string message;

		if (!getPrefix().empty()) {
			message += ":" + getPrefix() + " ";
		}

		message += getCommand();

		for (int i = 0; i < getParams().size(); ++i) {
			if (i == getParams().size() - 1) {
				message += " :" + getParams()[i] + "\r\n";
			} else {
				message += " " + getParams()[i];
			}
		}
		if (getParams().empty()) {
			message += "\r\n";
		}
		return message;
	}

	void IRC::Command::exec(Client & client, ListenSocket & server) const {
		if (!valid) {
			return;
		}
		std::map<std::string, cmd> const& commands = server.getCommands();
		cmd_const_iter it = commands.find(command);
		if (!client.isFlag(UMODE_REGISTERED) && it == commands.end()) {
			return;
		} else if (client.isFlag(UMODE_REGISTERED) && it == commands.end()) {
			sendError(client, server, ERR_UNKNOWNCOMMAND, command);
		} else if (!(client.isFlag(UMODE_REGISTERED)) && (it->first == CMD_PASS || it->first == CMD_NICK || it->first == CMD_USER)) {
			it->second(*this, client, server);
		} else if (!(client.isFlag(UMODE_REGISTERED)) && it != commands.end()) {
			sendError(client, server, ERR_NOTREGISTERED);
		} else if (client.isFlag(UMODE_REGISTERED)) {
			it->second(*this, client, server);
			std::cout << "[DEBUG] executed: " << command << std::endl;
		}
	}

	Command &operator<<(Command & command, std::string const& arg) {
		command.getParams().push_back(arg);
		return command;
	}

}