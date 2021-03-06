//
// Created by Frey Tinkerer on 11/11/21.
//
#pragma once

#ifndef FT_IRC_COMMAND_HPP
#define FT_IRC_COMMAND_HPP

#include "Client.hpp"
#include <string>
#include <vector>

//#include "commands.hpp"

namespace IRC {

	class ListenSocket;
	class Client;

	class Command {
	private:
		std::string prefix;
		std::string command;
		std::vector<std::string> params;
		bool valid;

	public:
		Command(std::string const &message);
		Command(const std::string &prefix, const std::string &command, const std::vector<std::string> &params);
//		Command(const std::string &prefix, const std::string &command);
		Command(const std::string &prefix, const std::string &command,
				const std::string &arg1 = "", const std::string &arg2 = "",
				const std::string &arg3 = "", const std::string &arg4 = "");

		virtual ~Command();

		const std::string &getPrefix() const;
		const std::string &getCommand() const;
		const std::vector<std::string> &getParams() const;
		std::vector<std::string> &getParams();
		std::string to_string() const;

		bool isValid() const;

		void exec(Client &client, ListenSocket &server) const;

//		void send_to(Client const &client, ListenSocket const &server) const;
//		void send_to(std::string const &nick, ListenSocket const &server) const;
	};

	Command &operator<<(Command & command, std::string const& arg);

}

#endif //FT_IRC_COMMAND_HPP
