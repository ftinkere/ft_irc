#ifndef LISTENINGSOCKET_HPP
#define LISTENINGSOCKET_HPP

#include "Socket.hpp"
#include <map>
#include <vector>
#define BUFFER_SIZE 256

namespace IRC{
    class ListenSocket : public Socket
    {
    public:
        ListenSocket( const char* port);
        ~ListenSocket();
        //typedef struct info_client client;
    private:
        fd_set read_fds;
//        fd_set master;
//        int fd_max;
        int fd_new;
        char buf[BUFFER_SIZE];
        char *ipv4;
        void check_connections(void);
        in_addr &get_in_addr(struct sockaddr *sa);
        char* recieve_ip(struct sockaddr_storage &remoteaddr);
        void new_client();
        void handle_chat(int &i);

        class SUBD
        {
            //typedef struct info_client client;
        private:
            struct client{
                std::string ipv4;
                std::string nick;
                std::string password;
                std::string user;
                int  fds;
                client() {};
            };
        public:
            SUBD() {};
            ~SUBD() {};
            client cl;
            std::pair<int, client> val;
            std::map<int, client > base_fd; // создать базу ников, имен и фдешников
            std::map<std::string, client>  base_nick;
            std::map<std::string, client>  base_user;
            int handle_message(const char *buf);
        };
        SUBD *base;
        
    };
}
#endif