#pragma once

#include <set>
#include <map>
#include <string>

#include "Client.hpp"
#include "commands.hpp"

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
namespace IRC{

	std::vector<std::string> split(const std::string &s, char delim);

	class Channel {
	private:
		std::string	name;
		int			flags;

	public:
		std::string	topic;
		std::string	password;
		int			limit;

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

		void add_memeber(Client & member);
		bool check_limit();

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
	};
}