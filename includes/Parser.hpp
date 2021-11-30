//
// Created by Frey Tinkerer on 11/11/21.
//

#ifndef FT_IRC_PARSER_HPP
#define FT_IRC_PARSER_HPP

namespace IRC {

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

// #define CMD_WHO "WHO"
#define CMD_WHOIS "WHOIS"
#define CMD_WHOWAS "WHOWAS" //нет

#define CMD_KILL "KILL"
#define CMD_PING "PING" //нет
#define CMD_PONG "PONG" //нет

//#define CMD_VERSION "VERSION"
//#define CMD_TIME "TIME"
#define CMD_ADMIN "ADMIN"
//#define CMD_INFO "INFO"

#define CMD_AWAY "AWAY"
//#define CMD_REHASH "REHASH"
//#define CMD_RESTART "RESTART"
#define CMD_WALLOPS "WALLOPS" // нет

}
#endif //FT_IRC_PARSER_HPP
