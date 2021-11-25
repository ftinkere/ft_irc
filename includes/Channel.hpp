#pragma once

#include <set>
#include <map>
#include <string>

#include "commands.hpp"

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
		std::string	topic;
		static std::vector<std::string> &split(const std::string &s, char delim, std::vector<std::string> &elems);
	public:
		std::string	password;
		int			limit;

		std::set<Client*>			users;
		std::set<std::string const*>	voiced;
		std::set<std::string const*>	opers;

		Channel(std::string const& name);
		virtual ~Channel() {};

		static bool check_name(std::string const& name);
		static std::vector<std::string> split(const std::string &s, char delim);

		void setFlag(int flag) { flags = flags | flag; }
		int getFlags() const { return flags; }
		bool isFlag(int flag) const { return flags & flag; }

		std::string const& getKey() const { return password; }

        std::string const& getTopic() const { return topic; }
        void setTopic(const std::string & cl)  { topic = cl; }
        void clearTopic()  { topic.clear(); }

		void add_memeber(Client & member);
		bool check_limit();



		std::string get_names() const;

        void erase_client(Client &cl);

        Channel();
    };
}