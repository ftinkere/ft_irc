#pragma once

#include <set>
#include <map>
#include <string>
#include <vector>

#include "commands.hpp"

#define CMODE_INVITE 0x01 // +i
#define CMODE_MODER 0x02      // +m
#define CMODE_SECRET 0x04      // +s
#define CMODE_OPER 0x08    // +o
#define CMODE_SPEAK 0x10    // +v
#define CMODE_TOPIC 0x20    // +t

#define UCMODE_INVIS 0x01    // +i
#define UCMODE_NOTIC 0x02    // +s
#define UCMODE_OPER 0x04    // +o
#define UCMODE_WALL 0x08    // +w
#define UCMODE_VECHE 0x10    // +v

#define CHAN 0    // name channels
#define KEY 1 // name KEY
#define ERR_NAME -1 // error name

//проверка названия канала
namespace IRC{

	class Channel {
	private:
		std::string	name;
		int			flags;
		std::string	topic;
	public:
		std::string	password;
		int			limit;

		std::set<Client const*>			voiced;
		std::set<std::string const*>	voiced;
		std::set<std::string const*>	opers;

		Channel(std::string const& name);
		virtual ~Channel() {};

		static bool check_name(std::string const& name);
		static std::vector<std::string> &get_args(std::string const& name);
		static std::vector<std::string> split(const std::string &s, char delim);
		static std::vector<std::string> &split(const std::string &s, char delim, std::vector<std::string> &elems);

		void setFlag(int flag) { flags = flags | flag; }
		int getFlags() const { return flags; }
		bool isFlag(int flag) const { return flags & flag; }

		std::string const& getKey() const { return password; }
		std::string const& getTopic() const { return topic; }
		void setTopic(std::string &flag) { topic = flag; }
		void clearTopic() { topic.clear(); }

		void add_memeber(Client const& member);
		bool check_limit();

		std::string get_names() const;

	};
}