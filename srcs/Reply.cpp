//
// Created by Luci Atkins on 11/20/21.
//

#include <iomanip>
#include "Reply.hpp"
#include <sstream>

namespace IRC {

	void sendError(const Client &user, ListenSocket const& server, int err, const std::string &arg1, const std::string &arg2) {
//		std::string msg = ":" + server.getServername() + " ";
		std::stringstream ss;
		ss << err;
//		msg += ss.str();

		Command cmd(server.getServername(), ss.str(), user.getNick());

		switch (err) {
			case ERR_NOSUCHNICK:
				cmd << arg1 << "No such nick/channel";
				break;
			case ERR_NOSUCHSERVER:
				cmd << arg1 << "No such server";
				break;
			case ERR_NOSUCHCHANNEL:
				cmd << arg1 << "No such channel";
				break;
			case ERR_CANNOTSENDTOCHAN:
				cmd << arg1 << "Cannot send to channel";
				break;
			case ERR_TOOMANYCHANNELS:
				cmd << arg1 << "You have joined too many channels";
				break;
			case ERR_WASNOSUCHNICK:
				cmd << arg1 << "There was no such nickname";
				break;
			case ERR_TOOMANYTARGETS:
				cmd << arg1 << "Duplicate recipients. No arg1 delivered";
				break;
			case ERR_NOORIGIN:
				cmd << "No origin specified";
				break;
			case ERR_NORECIPIENT:
				cmd << ("No recipient given (" + arg1 + ")");
				break;
			case ERR_NOTEXTTOSEND:
				cmd << "No text to send";
				break;
			case ERR_NOTOPLEVEL:
				cmd << arg1 << "No toplevel domain specified";
				break;
			case ERR_WILDTOPLEVEL:
				cmd << arg1 << "Wildcard in toplevel domain";
				break;
			case ERR_UNKNOWNCOMMAND:
				cmd << arg1 << "Unknown command";
				break;
			case ERR_NOMOTD:
				cmd << "MOTD File is missing";
				break;
			case ERR_NOADMININFO:
				cmd << arg1 << "No administrative info available";
				break;
			case ERR_FILEERROR:
				cmd << ("File error doing " + arg1 + " on " + arg2 + "");
				break;
			case ERR_NONICKNAMEGIVEN:
				cmd << "No nickname given";
				break;
			case ERR_ERRONEUSNICKNAME:
				cmd << arg1 << "Erroneus nickname";
				break;
			case ERR_NICKNAMEINUSE:
				cmd << arg1 << "Nickname is already in use";
				break;
			case ERR_NICKCOLLISION:
				cmd << arg1 << "Nickname collision KILL";
				break;
			case ERR_USERNOTINCHANNEL:
				cmd << arg1 << arg2 << "They aren't on that channel";
				break;
			case ERR_NOTONCHANNEL:
				cmd << arg1 << "You're not on that channel";
				break;
			case ERR_USERONCHANNEL:
				cmd << arg1 << arg2 << "is already on channel";
				break;
			case ERR_NOLOGIN:
				cmd << arg1 << "User not logged in";
				break;
			case ERR_SUMMONDISABLED:
				cmd << "SUMMON has been disabled";
				break;
			case ERR_USERSDISABLED:
				cmd << "USERS has been disabled";
				break;
			case ERR_NOTREGISTERED:
				cmd << "You have not registered";
				break;
			case ERR_NEEDMOREPARAMS:
				cmd << arg1 << "Not enough parameters";
				break;
			case ERR_ALREADYREGISTRED:
				cmd << "You may not reregister";
				break;
			case ERR_NOPERMFORHOST:
				cmd << "Your host isn't among the privileged";
				break;
			case ERR_PASSWDMISMATCH:
				cmd << "Password incorrect";
				break;
			case ERR_YOUREBANNEDCREEP:
				cmd << "You are banned from this server";
				break;
			case ERR_KEYSET:
				cmd << arg1 << "Channel key already set";
				break;
			case ERR_CHANNELISFULL:
				cmd << arg1 << "Cannot join channel (+l)";
				break;
			case ERR_UNKNOWNMODE:
				cmd << arg1 << "is unknown mode char to me";
				break;
			case ERR_INVITEONLYCHAN:
				cmd << arg1 << "Cannot join channel (+i)";
				break;
			case ERR_BANNEDFROMCHAN:
				cmd << arg1 << "Cannot join channel (+b)";
				break;
			case ERR_BADCHANNELKEY:
				cmd << arg1 << "Cannot join channel (+k)";
				break;
			case ERR_BADCHANMASK:
				cmd << arg1 << "Invalid channel name";
				break;
			case ERR_NOPRIVILEGES:
				cmd << "Permission Denied. You're not an IRC operator";
				break;
			case ERR_CHANOPRIVSNEEDED:
				cmd << arg1 << "You're not channel operator";
				break;
			case ERR_CANTKILLSERVER:
				cmd << "You cant kill a server!";
				break;
			case ERR_NOOPERHOST:
				cmd << "No O-lines for your host";
				break;
			case ERR_UMODEUNKNOWNFLAG:
				cmd << "Unknown MODE flag";
				break;
			case ERR_USERSDONTMATCH:
				cmd << "Cant change mode for other users";
				break;
			default:
				return;
		}
//		send(user.getFd(), msg.c_str(), msg.size(), 0);
		server.send_command(cmd, user);
	}

	void sendReply(const Client &user, ListenSocket const& server, int rpl,
				   const std::string &arg1, const std::string &arg2,
				   const std::string &arg3, const std::string &arg4,
				   const std::string &arg5, const std::string &arg6,
				   const std::string &arg7, const std::string &arg8) {
		std::stringstream ss;
		ss << std::setfill('0') << std::setw(3) << rpl;

		Command cmd(server.getServername(), ss.str(), user.getNick());

		switch (rpl) {
			case RPL_WELCOME:
				cmd << ("Welcome to the FT Network, " + user.get_full_name());
				break;
			case RPL_YOURHOST:
				cmd << ("Your host is " + server.getServername() + ", running version " + SERV_VERSION);
				break;
			case RPL_CREATED: {
				struct tm time = *localtime(&server.getCreationTime());
				char buf[100] = {0};
				strftime(buf, 99, "%d-%m-%Y %H-%M-%S", &time);
				cmd << ("This server was created " + std::string(buf));
				break;
			}
			case RPL_MYINFO:
				cmd << server.getServername() << SERV_VERSION << "iow" << "imnst" << "kl";
				break;
			case RPL_ISUPPORT:
				cmd << "CASEMAPPING=ascii" << "CHANMODES=,k,l,imnst"
				<< "CHANTYPES=#" << "PREFIX=(ov)@+" << "are supported by this server";
				break;
			case RPL_USERHOST: {
				cmd << arg1;
				break;
			}
			case RPL_ISON:
				cmd << arg1;
				break;
			case RPL_AWAY:
				cmd << arg1 << arg2;
				break;
			case RPL_UNAWAY:
				cmd << "You are no longer marked as being away";
				break;
			case RPL_NOWAWAY:
				cmd << "You have been marked as being away";
				break;
			case RPL_WHOISUSER:
				cmd << arg1 << arg2 << arg3 << arg4;
				break;
			case RPL_WHOISSERVER:
				cmd << arg1 << arg2 << arg3;
				break;
			case RPL_WHOISOPERATOR:
				cmd << arg1 << "is an IRC operator";
				break;
			case RPL_WHOISIDLE:
				cmd << arg1 << arg2 << arg3 << "seconds idle";
				break;
			case RPL_ENDOFWHOIS:
				cmd << arg1 << "End of /WHOIS list";
				break;
			case RPL_WHOISCHANNELS:
				cmd << arg1 << arg2;
				break;
			case RPL_WHOWASUSER:
				cmd << arg1 << arg2 << arg3 << arg4;
				break;
			case RPL_ENDOFWHOWAS:
				cmd << arg1 << "End of WHOWAS";
				break;
			case RPL_LISTSTART:
				cmd << "Channel" << "Users  Name";
				break;
			case RPL_LIST:
				cmd << arg1 << arg2 << arg3;
				break;
			case RPL_LISTEND:
				cmd << "End of /LIST";
				break;
			case RPL_CHANNELMODEIS:
				cmd << arg1 << ("+" + arg2);
				if (!arg3.empty())
					cmd << arg3;
				if (!arg4.empty())
					cmd << arg4;
				break;
			case RPL_NOTOPIC:
				cmd << arg1 << "No topic is set";
				break;
			case RPL_TOPIC:
				cmd << arg1 << arg2;
				break;
			case RPL_INVITING:
				cmd << arg1 << arg2;
				break;
			case RPL_SUMMONING:
				cmd << arg1 << "Summoning user to IRC";
				break;
			case RPL_VERSION:
				cmd << (arg1 + "." + arg2) << arg3 << arg4;
				break;
			case RPL_WHOREPLY:
				cmd << arg1 << arg2 << arg3 << arg4
					<< arg5 << arg6 << arg7 << arg8;
				break;
			case RPL_ENDOFWHO:
				cmd << arg1 << "End of /WHO list";
				break;
			case RPL_NAMREPLY:
				cmd << arg1 << arg2 << arg3 ;
				break;
			case RPL_ENDOFNAMES:
				cmd << arg1 << "End of /NAMES list";
				break;
			case RPL_LINKS:
				cmd << arg1 << arg2 << (arg3 + " " + arg4);
				break;
			case RPL_ENDOFLINKS:
				cmd << arg1 << "End of /LINKS list";
				break;
			case RPL_BANLIST:
				cmd << arg1 << arg2;
				break;
			case RPL_ENDOFBANLIST:
				cmd << arg1 << "End of channel ban list";
				break;
			case RPL_INFO:
				cmd << arg1;
				break;
			case RPL_ENDOFINFO:
				cmd << "End of /INFO list";
				break;
			case RPL_MOTDSTART:
				cmd << (arg1 + " Message of the day");
				break;
			case RPL_MOTD:
				cmd << arg1;
				break;
			case RPL_ENDOFMOTD:
				cmd << "End of /MOTD command";
				break;
			case RPL_YOUREOPER:
				cmd << "You are now an IRC operator";
				break;
			case RPL_REHASHING:
				cmd << arg1 << "Rehashing";
				break;
			case RPL_TIME:
				cmd << arg1 << arg2; // ctime return string following ''
				break;
			case RPL_USERSSTART:
				cmd << "UserID   Terminal  Host"; // ????, ?????? ??????
				break;
			case RPL_USERS:
				cmd << "%-8s %-9s %-8s"; // ????, ?????? ??????
				break;
			case RPL_ENDOFUSERS:
				cmd << "End of users";
				break;
			case RPL_NOUSERS:
				cmd << "Nobody logged in";
				break;
//			case RPL_TRACELINK:
//				cmd << "Link" << arg1 << arg2 << arg3;
//				break;
//			case RPL_TRACECONNECTING:
//				cmd << "Try." << arg1 << arg2;
//				break;
//			case RPL_TRACEHANDSHAKE:
//				cmd << "H.S." << arg1 << arg2;
//				break;
//			case RPL_TRACEUNKNOWN:
//				cmd << "????" << arg1 << arg2; // ????
//				break;
//			case RPL_TRACEOPERATOR:
//				cmd << "Oper" << arg1 << arg2;
//				break;
//			case RPL_TRACEUSER:
//				cmd << "User" << arg1 << arg2;
//				break;
//			case RPL_TRACESERVER:
//				cmd << "Serv" << arg1 << (arg2 + "S") << (arg3 + "C");
//				cmd << arg4 << (arg5 + "@") << arg6;
//				break;
//			case RPL_TRACENEWTYPE:
//				cmd << arg1 << "0" << arg2;
//				break;
//			case RPL_TRACELOG:
//				cmd << "File" << arg1 << arg2;
//				break;
			case RPL_STATSLINKINFO:
				cmd << arg1 << arg2 << arg3 << arg4
					<< arg5 << arg6 << arg7;
				break;
			case RPL_STATSCOMMANDS:
				cmd << arg1 << arg2;
				break;
			case RPL_STATSCLINE:
				cmd << "C" << arg1 << "*" << arg2 << arg3 << arg4;
				break;
			case RPL_STATSNLINE:
				cmd << "N" << arg1 + "*" << arg2 << arg3 << arg4;
				break;
			case RPL_STATSILINE:
				cmd << "I" << arg1 << "*" << arg2 << arg3 << arg4;
				break;
			case RPL_STATSKLINE:
				cmd << "K" << arg1 << "*" << arg2 << arg3 << arg4;
				break;
			case RPL_STATSYLINE:
				cmd << "Y" << arg1 << "*" << arg2 << arg3 << arg4;
				break;
			case RPL_ENDOFSTATS:
				cmd << arg1 << "End of /STATS report";
				break;
			case RPL_STATSLLINE:
				cmd << "L" << arg1 << "*" << arg2 << arg3;
				break;
			case RPL_STATSUPTIME:
				cmd << ("Server Up " + arg1 + " days " + arg2 + ":" + arg3 + ":" + arg4);
				break;
			case RPL_STATSOLINE:
				cmd << "O" << arg1 << "*" << arg2;
				break;
			case RPL_STATSHLINE:
				cmd << "H" << arg1 << "*" << arg2;
				break;
			case RPL_UMODEIS:
				cmd << arg1;
				break;
			case RPL_LUSERCLIENT: {
				size_t users_c = 0;
				size_t inv_c = 0;
				for (client_const_iter it = server.clients.begin(); it != server.clients.end(); ++it) {
					if (it->isFlag(UMODE_REGISTERED)) ++users_c;
					if (it->isFlag(UMODE_INVIS)) ++inv_c;
				}
				std::stringstream smsg;
				smsg << "There are " << users_c << " users and " << inv_c << " invisible on 1 servers";
				cmd << smsg.str();
				break;
			}
			case RPL_LUSEROP: {
				size_t i_c = 0;
				for (client_const_iter it = server.clients.begin(); it != server.clients.end(); ++it) {
					if (it->isFlag(UMODE_OPER)) ++i_c;
				}
				std::stringstream smsg;
				smsg << i_c;
				cmd << smsg.str() << "operator(s) online";
				break;
			}
			case RPL_LUSERUNKNOWN: {
				size_t i_c = 0;
				for (client_const_iter it = server.clients.begin(); it != server.clients.end(); ++it) {
					if (!it->isFlag(UMODE_REGISTERED)) ++i_c;
				}
				std::stringstream smsg;
				smsg << i_c;
				cmd << smsg.str() << "unknown connection(s)";
				break;
			}
			case RPL_LUSERCHANNELS: {
				std::stringstream smsg;
				smsg << server.channels.size();
				cmd << smsg.str() << "channels formed";
				break;
			}
			case RPL_LUSERME: {
				std::stringstream smsg;
				smsg << server.clients.size();
				cmd << ("I have " + smsg.str() + " clients and 1 servers");
				break;
			}
			case RPL_ADMINME:
				cmd << arg1 << "Administrative info";
				break;
			case RPL_ADMINLOC1:
				cmd << ("Name     " + arg1);
				break;
			case RPL_ADMINLOC2:
				cmd << ("Nickname " + arg1);
				break;
			case RPL_ADMINEMAIL:
				cmd << ("E-Mail   " + arg1);
				break;
			default:
				cmd << "UNKNOWN REPLY";
//				break;
				return;
		}
		server.send_command(cmd, user);
	}
}
