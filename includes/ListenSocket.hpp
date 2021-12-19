#pragma once

#ifndef LISTENINGSOCKET_HPP
#define LISTENINGSOCKET_HPP

#include "types.hpp"
#include "Channel.hpp"
#include "Command.hpp"
#include "Socket.hpp"
#include <list>
#include <map>
#include <vector>

#define BUFFER_SIZE 1025

#define SERV_VERSION "0.0.pre1"

namespace IRC {

	class ListenSocket : public Socket {
	public:
		ListenSocket(const char *port);
		virtual ~ListenSocket();

		void execute();
		void configure(std::string const &path);
		void set_password(std::string const &password);


		std::list<Client> clients;
		std::map<std::string, Channel> channels;
		std::map<std::string, std::string> opers;
		std::map<std::string, std::string> admin;
		std::multimap<std::string, Client> base_client;


	private:
		fd_set read_fds;
		std::string servername;
		std::string password;

		std::map<std::string, cmd> commands;

		time_t creation_time;

		void check_connections(void);
		static in_addr &get_in_addr(struct sockaddr *sa);
		char *recieve_ip(struct sockaddr_storage &remoteaddr);
		void new_client();
		void handle_chat(int const &fd);
		int handle_message(const char *buf, Client *client);

	public:
		const static time_t registration_timeout = 60;
		const static time_t ping_period = 240;
		const static time_t pong_timeout = 240;

		const std::list<Client> &getClients() const;
		const fd_set &getReadFds() const;
		const std::string &getServername() const;
		const std::string &getPassword() const;
		const std::map<std::string, cmd> &getCommands() const;
		const time_t &getCreationTime() const { return creation_time; }
		void quit_client(int fd);

		bool isChannelExist(std::string const &chan) const;
		bool isChannelExist(channel_iter const &chan) const;
		bool isChannelExist(channel_const_iter const &chan) const;
		void addNewChannel(std::string const &chan);
		channel_iter getChannel(std::string const &chan);

		client_iter getClient(std::string const &nick);
		client_iter getClient(int fd);
		client_const_iter getClient(std::string const &nick) const;
		client_const_iter getClient(int fd) const;
		client_iter getClientByMask(const std::string &mask);
		client_const_iter getClientByMask(const std::string &mask) const;

		bool isClientExist(std::string const &nick) const;
		bool isClientExist(client_iter const &it) const;

		Client *thisisnick(const std::string &nick, int flag, Client &feedback);

		void send_command(Command const &command, Client const &client) const;
		void send_command(Command const &command, std::string const &nickname) const;
		void send_command(Command const &command, int fd) const;
		void send_command(Client const &client, std::string const &cmd,
						  std::string const &arg1 = "", std::string const &arg2 = "",
						  std::string const &arg3 = "", std::string const &arg4 = "") const;
		void send_command(int fd, std::string const &cmd,
						  std::string const &arg1 = "", std::string const &arg2 = "",
						  std::string const &arg3 = "", std::string const &arg4 = "") const;

		channel_const_iter getChannel(const std::string &chan) const;
	};
}// namespace IRC
#endif