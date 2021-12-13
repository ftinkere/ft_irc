#pragma once

#ifndef IRC_CHANNEL_HPP
#define IRC_CHANNEL_HPP

#include "ListenSocket.hpp"
#include "Client.hpp"
#include <map>
#include <set>
#include <string>
#include <vector>

#define CMODE_INVITE 0x01 // +i
#define CMODE_MODER 0x02      // +m
#define CMODE_SECRET 0x04      // +s
#define CMODE_NOEXT 0x08    // +n
#define CMODE_TOPIC 0x10    // +t

#define INVIT 'i'
#define MODES 'm'
#define SECRET 's'
#define SPEAK 'n'
#define TOPIC 't'
#define OPER 'o'
#define VOICER 'v'
#define KEY 'k'
#define LEN 'l'

//проверка названия канала
namespace IRC {
	class Client;

	std::vector<std::string> split(const std::string &s, char delim);

	class Channel {
	private:
		std::string	name;
		int			flags;
		std::string	topic;
		std::string	password;
		int			limit;

		bool to_delete;

	public:

		std::set<Client *>				users;
		std::set<std::string const*>	voiced;
		std::set<std::string const*>	opers;

		Channel();
		Channel(std::string const& name);
		virtual ~Channel() {};

		static bool check_name(std::string const& name);

		const std::string& getName() const { return name; };

		void setFlag(int flag) { flags = flags | flag; }
		void zeroFlag(int flag) { flags &= ~flag; }
		int getFlags() const { return flags; }
		bool isFlag(int flag) const { return flags & flag; }

		std::string const& getKey() const { return password; }
		void setKey(std::string const& key) { password = key; }
		void clearKey() { password.clear(); }

		int const& getLimit() const { return limit; }
		void setLimit(int const& key) { limit = key; }
		void clearLimit() { limit = 0; }

		std::string const& getTopic() const { return topic; }
		void setTopic(const std::string & cl)  { topic = cl; }
		void clearTopic()  { topic.clear(); }

		bool isClient(Client const& client) const;
		bool isClient(client_iter const& client) const;
		bool isOper(Client const& client) const;
		bool isOper(client_iter const& client) const;
		bool isVoiced(Client const& client) const;
		bool isVoiced(client_iter const& client) const;
		void add_memeber(Client & member);
		bool check_limit() const;

		std::string get_names() const;

		void erase_client(Client &cl);

		enum model {
			I,
			M,
			S,
			N,
			T,
			O,
			V,
			K,
			L
		};

		static std::map<const char, enum model> modes;

		bool isOper(const std::string &nick) const;

		bool isOper(const channel_client_iter &client) const;

		bool isVoiced(const channel_client_iter &client) const;

		bool isVoiced(const std::string &nick) const;

		bool isDelete();
	};
}

#endif