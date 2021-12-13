//
// Created by ftinkere on 08.12.2021.
//
#pragma once

#ifndef FT_IRC_TYPES_HPP
#define FT_IRC_TYPES_HPP

#include <list>
#include <map>
#include <set>
#include <string>
namespace IRC {
	class Client;
	class Command;
	class Channel;
	class ListenSocket;
	typedef void (*cmd)(Command const &, Client &, ListenSocket &);
	typedef std::list<Client>::iterator client_iter;
	typedef std::list<Client>::const_iterator client_const_iter;
	typedef std::map<std::string, Channel>::iterator channel_iter;
	typedef std::map<std::string, Channel>::const_iterator channel_const_iter;
	typedef std::map<std::string, std::string>::iterator oper_iter;
	typedef std::map<std::string, std::string>::iterator admin_iter;
	typedef std::multimap<std::string, Client>::iterator base_client_iter;
	typedef std::map<std::string, cmd>::const_iterator cmd_const_iter;
	typedef std::set<Client *>::iterator channel_client_iter;
	typedef std::set<const std::string*>::iterator channel_ov_iter;

}
#endif//FT_IRC_TYPES_HPP
