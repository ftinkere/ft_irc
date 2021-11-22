#pragma once

#include <set>
#include <map>
#include <string>

#include "Client.hpp"

#define CMODE_INVITE 0x01 // +i
#define CMODE_MODER 0x02      // +m
#define CMODE_SECRET 0x04      // +s
#define CMODE_NOEXT 0x08    // +n
#define CMODE_TOPIC 0x10    // +t

//#define CHAN 0    // name channels
//#define KEY 1 // name KEY
//#define ERR_NAME -1 // error name

//проверка названия канала
namespace IRC{

	class Channel {
	private:
		std::string	name;
		int			flags;
	public:
		std::string	topic;
		std::string	password;
		int			limit;

		std::set<Client const*>			users;
		std::set<std::string const*>	voiced;
		std::set<std::string const*>	opers;

		Channel(std::string const& name);
		virtual ~Channel() {};

		static bool check_name(std::string const& name);

		void setFlag(int flag) { flags = flags | flag; }
		int getFlags() const { return flags; }
		bool isFlag(int flag) const { return flags & flag; }

		std::string const& getKey() const { return password; }

		void add_memeber(Client const& member);
		bool check_limit();

		std::string get_names() const;

	};
}