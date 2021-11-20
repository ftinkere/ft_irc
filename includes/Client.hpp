//
// Created by Frey Tinkerer on 11/11/21.
//

#ifndef FT_IRC_CLIENT_HPP
#define FT_IRC_CLIENT_HPP

#include <string>

#define UMODE_REGISTERED 0x01 // +r
#define UMODE_NOPER 0x02      // +o
#define UMODE_INVIS 0x04      // +i
#define UMODE_WALLOPS 0x08    // +w

namespace IRC {

	class ListenSocket;

	class Client {
	private:
//		std::string pass;
//		std::string nick;
//		std::string user;
		int fd;
		int flags;

	public:
		std::string pass;
		std::string nick;
		std::string user;

		std::string host;

		time_t login_time;

		Client(int fd) : fd(fd), flags(0) {}

		virtual ~Client() {}

		bool try_register(ListenSocket & server);

		// 0b00000001 // flags
		// 0b00000010 // flag
		void setFlag(int flag) { flags = flags | flag; }
		// if (getFlags() & UMODE_REGISTERED)
		int getFlags() const { return flags; }
		const std::string &getNick() const { return nick; }
		void setNick(const std::string &nick) { Client::nick = nick; }
		const std::string &getUser() const { return user; }
		void setUser(const std::string &user) { Client::user = user; }
		int getFd() const { return fd; }
		bool _name_control(std::string &prefix, int v);
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

	// find_if(.., .., is_nickname("nick"));
	is_nickname_s is_nickname(std::string nickname);
	is_fd_s is_fd(int fd);



}
#endif //FT_IRC_CLIENT_HPP
