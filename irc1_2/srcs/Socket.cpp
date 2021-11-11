#include "Socket.hpp"

namespace IRC{
    Socket::Socket(const char* port)
    {
        int rv;
        struct addrinfo hints, *ai, *bindp;

        FD_ZERO(&master); //очищаем сет
        // получаем сокет и биндим его
        memset(&hints, 0, sizeof hints);
        hints.ai_family = AF_INET;
        hints.ai_socktype = SOCK_STREAM;
        hints.ai_flags = AI_PASSIVE;
        if ((getaddrinfo(NULL, port, &hints, &ai)) != 0) {
            throw std::runtime_error("Selected server: getaddrinfo\n");
        }
        socket_bind(1, ai, bindp);
        setup_listen();
    }

    void Socket::socket_bind(int yes, struct addrinfo *ai, struct addrinfo *bindp)
    {
        for(bindp = ai; bindp != NULL; bindp = bindp->ai_next) {
            listener = socket(bindp->ai_family, bindp->ai_socktype, bindp->ai_protocol);
            if (listener < 0) {
                continue;
            }

        // избегаем ошибки «address already in use»
            setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));

            if (bind(listener, bindp->ai_addr, bindp->ai_addrlen) < 0) {
                close(listener);
                continue;
            }
            break;
        }
        if (bindp == NULL) {
            throw std::runtime_error("Selected server: failed to bind\n");
        }
        freeaddrinfo(ai); // с этим мы всё сделали
    }

    void Socket::setup_listen()
    {
            // слушаем
        if (listen(listener, SOMAXCONN) == -1) {
            throw std::runtime_error("Listen\n");
        }

    // добавляем слушающий сокет в мастер-сет
        FD_SET(listener, &master);

        // следим за самым большим номером дескриптора
        fd_max = listener; // на данный момент это этот
    }

    // getter functions
    int &Socket::getListener()
    {
        return listener;
    }
}