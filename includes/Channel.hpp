#pragma once

#include <set>
#include <map>
#include <string>

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

    class Client;
class Channel
{
    public:
        Channel();
        static int check_name(std::string &_name);
        void setFlag(int flag) { flags = flags | flag; }
		// if (getFlags() & UMODE_REGISTERED)
		int getFlags() const { return flags; }
        void setName(std::string &_name) { name = _name; }
        std::string & getKey() { return password; }
        void add_memeber(const std::string  &member);
        bool check_limit();
//        int getLimit() const { return limit; }
    private:
        std::map<std::string, int> base;
        std::string name;
        int flags;
        std::string topic;
        std::string password;
        int limit;

};
}