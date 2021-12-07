//
// Created by Frey Tinkerer on 11/11/21.
//

#ifndef FT_IRC_CLIENT_HPP
#define FT_IRC_CLIENT_HPP

#include <string>
#include <list>
#include "commands.hpp"

#define UMODE_REGISTERED 0x01 // +r
#define UMODE_OPER 0x02       // +o
#define UMODE_INVIS 0x04      // +i
#define UMODE_WALLOPS 0x08    // +w

namespace IRC {

	class ListenSocket;
//	class Command;

	class Client {
	private:
//		std::string pass;
//		std::string nick;
//		std::string user;
		int fd;
		int flags;
		std::list<std::string> channels;
		std::string pass;
		std::string nick;
		std::string user;
		std::string realname;
		std::string away;
		std::string host;
		bool to_disconnect;
		time_t login_time;
		time_t last_pingpong;
		bool pinged;
	public:

		Client() : fd(-1), flags(UMODE_WALLOPS), to_disconnect(false), pinged(false) {}
		Client(int fd) : fd(fd), flags(UMODE_WALLOPS), to_disconnect(false), pinged(false) {}

		virtual ~Client() {}

		bool try_register(ListenSocket & server);

		// 0b00000001 // flags
		// 0b00000010 // flag
		void setFlag(int flag) { flags = flags | flag; }
		void zeroFlag(int flag) { flags &= ~flag; }
		// if (getFlags() & UMODE_REGISTERED)
		int getFlags() const { return flags; }
		int isFlag(int flag) const { return flags & flag; }
		void disconnect() { this->to_disconnect = true; }

		const std::string &getHost() const { return host; }
		void setHost(const std::string &host) { this->host = host; }

		const std::string &getNick() const { return nick; }
		void setNick(const std::string &nick) { Client::nick = nick; }

		const std::string &getPass() const { return pass; }
		void setPass(const std::string &pass) { this->pass = pass; }

		const std::string &getUser() const { return user; }
		void setUser(const std::string &user) { Client::user = user; }

		void setAway(const std::string &msg) { Client::away = msg; }
		const std::string &getAway() const { return away; }
		void clearAway() { Client::away.clear(); }

		const time_t getTime() const { return login_time; }

		std::list<std::string> &getChannels() { return channels; }
		void addChannel(std::string const& flag) { channels.push_back(flag); }
		void eraseChannel(std::string const& flag);

		int getFd() const { return fd; }

		bool _name_control(std::string const& prefix, int v);
		std::string get_full_name() const;
	};

	struct is_nickname_s {
		std::string nickname;

		explicit is_nickname_s(std::string nickname) : nickname(nickname) {};

		bool operator()(Client const& c) const { return nickname == c.getNick(); }
	};

	struct is_fd_s {
		int fd;

		explicit is_fd_s(int fd) : fd(fd) {};

		bool operator()(Client const& c) const { return fd == c.getFd(); }
	};

	struct is_flag_s {
		int flag;

		explicit is_flag_s(int flag) : flag(flag) {};

		bool operator()(Client const& c) const { return c.isFlag(flag); }
	};

	// find_if(.., .., is_nickname("nick"));
	is_nickname_s is_nickname(std::string nickname);
	is_fd_s is_fd(int fd);
	is_flag_s is_flag(int flag);



}
#endif //FT_IRC_CLIENT_HPP
