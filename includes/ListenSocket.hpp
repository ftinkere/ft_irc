#ifndef LISTENINGSOCKET_HPP
#define LISTENINGSOCKET_HPP

#include <map>
#include <vector>
#include <list>
#include "Socket.hpp"
//#include "Command.hpp"
//#include "Channel.hpp"
#include "commands.hpp"
#include "Reply.hpp"

#define WITMSG -1

#define BUFFER_SIZE 1025

namespace IRC {
    class Channel;
    class Command;
    class ListenSocket : public Socket
    {
    public:
        ListenSocket(const char* port);
        virtual ~ListenSocket();

		void execute();
		void configure(std::string const& path);
		void set_password(std::string const& password);

		typedef void (*cmd)(Command const&, Client&, ListenSocket&);
		std::list<Client> clients;
		std::map<std::string, Channel> channels;
		std::map<std::string, std::string> opers;
		std::map<std::string, std::string> admin;
    private:

		fd_set read_fds;
		std::string servername;
		std::string password;

		std::map<std::string, cmd> commands;

//        fd_set master;
//        int fd_max;
//        int fd_new;
//        char buf[BUFFER_SIZE];
//        char *ipv4;

        void check_connections(void);
        static in_addr &get_in_addr(struct sockaddr *sa);
        char* recieve_ip(struct sockaddr_storage &remoteaddr);
        void new_client();
        void handle_chat(int const& fd);
		int handle_message(const char *buf, Client *client);

	public:
		const static time_t registration_timeout = 30;

		const std::list<Client> &getClients() const;
		const fd_set &getReadFds() const;
		const std::string &getServername() const;
		const std::string &getPassword() const;
		const std::map<std::string, cmd> &getCommands() const;
		void quit_client(int fd);

		std::vector<Client*> find_clients(const std::string &nick, int flag, Client const& feedback);
		std::vector<Client*> find_clients(const std::string &nick, Client const& feedback);

		Client* thisisnick(const std::string &nick, int flag, Client& feedback);
		Channel* thisischannel(const std::string &nick, int flag, Client& feedback);

		void send_command(Command const& command, Client const& client);
		void send_command(Command const& command, std::string const& nickname);
		void send_command(Command const& command, int fd);

//        class SUBD
//        {
//            //typedef struct info_client client;
//        private:
//            struct client{
//                std::string ipv4;
//                std::string nick;
//                std::string password;
//                std::string user;
//                int  fds;
//                client() {};
//            };
//        public:
//            SUBD() {};
//            ~SUBD() {};
//
//
//            client cl;
//            std::pair<int, client> val;

//            std::map<int, client > base_fd; // создать базу ников, имен и фдешников
//            std::map<std::string, client>  base_nick;
//            std::map<std::string, std::string>  base_user;
//		base_user["LAN"] = "en_US.UTF-8";
//		std::pair<std::string, std::string> val;
//		val = (LC_TERMINAL=iTerm2);
//		map.insert(val);
//        };
//        SUBD *base;
//
    };
}
#endif