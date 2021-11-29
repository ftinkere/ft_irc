//
// Created by Frey Tinkerer on 11/11/21.
//

#ifndef FT_IRC_PARSER_HPP
#define FT_IRC_PARSER_HPP

#include <vector>
#include "Client.hpp"

namespace IRC {

#define SYM_COLON ':'

#define MAX_LEN_NICK 9

#define CMD_PASS "PASS"
#define CMD_NICK "NICK"
#define CMD_USER "USER"
#define CMD_OPER "OPER"
#define CMD_QUIT "QUIT"

#define CMD_JOIN "JOIN"
#define CMD_PART "PART"
#define CMD_MODE "MODE"
#define CMD_TOPIC "TOPIC"
#define CMD_NAMES "NAMES"
#define CMD_LIST "LIST"
#define CMD_INVITE "INVITE"
#define CMD_KICK "KICK"

#define CMD_PRIVMSG "PRIVMSG"
#define CMD_NOTICE "NOTICE"

//#define CMD_WHO "WHO"
//#define CMD_WHOIS "WHOIS"
//#define CMD_WHOWAS "WHOWAS"

#define CMD_KILL "KILL"
#define CMD_PING "PING"
#define CMD_PONG "PONG"

//#define CMD_VERSION "VERSION"
//#define CMD_TIME "TIME"
#define CMD_ADMIN "ADMIN"
//#define CMD_INFO "INFO"

#define CMD_AWAY "AWAY"
//#define CMD_REHASH "REHASH"
//#define CMD_RESTART "RESTART"
#define CMD_WALLOPS "WALLOPS"

#define ERROR_ 1
#define SUCSSES_ 0
#define TWO_WORDS -1

	class Parser {
	public:
		typedef std::vector<Client>& Base;
		Parser(Base clients, std::string buf, int fd);
		~Parser();
	private:
		std::vector<std::string> params;
		Client* id;
		int prefix_parsing(std::string &buf);// парсим первое слово
		int _parsing_command_word(std::string &buf, int &first_char);// парсим второе слово
		int _parsing_param_word(std::string &buf, int &first_char);// парсим третье слово
	};
}


#endif //FT_IRC_PARSER_HPP
