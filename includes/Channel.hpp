#pragma once

#include <set>

#define CMODE_INVITE 0x01 // +i
#define CMODE_MODER 0x02      // +m
#define CMODE_SECRET 0x04      // +s
#define CMODE_OPER 0x08    // +o
#define CMODE_SPEAK 0x10    // +s
#define CMODE_PASS 0x20    // +k
#define CMODE_LIMIT 0x40    // +l
#define CMODE_TOPIC 0x80    // +t

#define UCMODE_INVIS 0x01    // +i
#define UCMODE_NOTIC 0x02    // +s
#define UCMODE_OPER 0x04    // +o
#define UCMODE_WALL 0x08    // +w
#define UCMODE_VECHE 0x10    // +v

//проверка названия канала
namespace IRC{

    class Client;
class Channel
{
    public:
        Channel();
        void setFlag(int flag) { flags = flags | flag; }
		// if (getFlags() & UMODE_REGISTERED)
		int getFlags() const { return flags; }
    private:
        std::map<std::string, int> base;
        std::string name;
        int flags;
        std::string topic;
        std::string password;
        int limit;

};
}