//
// Created by Frey Tinkerer on 11/11/21.
//

#include <iostream>
#include <Client.hpp>
#include <ListenSocket.hpp>
#include "Parser.hpp"
#include "Command.hpp"


char to_upper(char& c) {
	c = (char)std::toupper((int)c);
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
		std::string buf;
		std::transform(command.begin(), command.end(), std::back_inserter(buf), &to_upper);
		pos = command_end + 1;
	}

	std::cout << "[DEBUG]: prefix: \"" << prefix << "\", command: \"" << command << "\", valid: \"" << valid << "\"" << std::endl;

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
		if (pos < message.length() && message[pos] != '\n' && message[pos] != '\r') {
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

IRC::Command::~Command() {}

const std::string &IRC::Command::getPrefix() const { return prefix; }
const std::string &IRC::Command::getCommand() const { return command; }
const std::vector<std::string> &IRC::Command::getParams() const { return params; }
bool IRC::Command::isValid() const { return valid; }



void IRC::Command::exec(Client & client, ListenSocket & server) const {
    std::map<std::string, ListenSocket::cmd>::const_iterator it;
	std::map<std::string, ListenSocket::cmd> const& commands = server.getCommands();
	it = commands.find(command);
	if (!(client.getFlags() & UMODE_REGISTERED) && (it->first == CMD_PASS || it->first == CMD_NICK || it->first == CMD_USER))
	{
	    it->second(*this, client, server);
	}
	else if (!(client.getFlags() & UMODE_REGISTERED) && it != commands.end())
	{
	    // if not registered - not registered 451 (except pass, nick, user)
	    sendError(client, server, 451, "", "");
	}
	else if (!(client.getFlags() & UMODE_REGISTERED) && it == commands.end())
	{
	    return;
	}
	else if (client.getFlags() & UMODE_REGISTERED && it == commands.end())
	{
	    // reply command not found 421
	    sendError(client, server, 421, command, "");
	}
	else if (client.getFlags() & UMODE_REGISTERED)
	{
		it->second(*this, client, server);
		std::cout << "|DEBUG| " << command << "!!!!" << std::endl;
	}
}

void IRC::Command::send_to(IRC::Client const& client, IRC::ListenSocket const& server) const {
	std::string message;

	if (!this->prefix.empty()) {
		message += ":" + server.getServername();
	} else {
		message += ":" + this->prefix;
	}

	message += " " + command;
	for (int i = 0; i < params.size(); ++i) {
		if (i == params.size() - 1) {
			message += " :" + params[i] + "\r\n";
		} else {
			message += " " + params[i];
		}
	}
	if (params.empty()) {
		message += "\r\n";
	}
	send(client.getFd(), message.c_str(), message.size(), 0);
}

void IRC::Command::send_to(const std::string &nick, IRC::ListenSocket const& server) const {
	std::vector<const Client>::iterator to = std::find_if(server.getClients().begin(), server.getClients().end(), is_nickname(nick));
	if (to == server.getClients().end()) {
		// reply nick not found
	} else {
		send_to(*to, server);
	}
}
