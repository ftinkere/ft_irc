#pragma once

#ifndef SOCKET_HPP
#define SOCKET_HPP

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <iostream>

namespace IRC
{
    class Socket
    {
        protected:
            int fd_max; // макс. число дескрипторов
            fd_set master; // главный сет дескрипторов
            int listener; // дескриптор слушающего сокета
            int &getListener();
        private:
            void socket_bind(int yes, struct addrinfo *ai, struct addrinfo *p);
            void setup_listen(void);
        public:
            Socket(const char* port);
            virtual ~Socket() {};

//            int &getFdmax();
//            fd_set &getMaster();
//            int &getListener();

    };
} // namespace IRC

#endif