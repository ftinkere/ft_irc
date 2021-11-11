#include "ListeningSocket.hpp"

namespace IRC{
    ListenSocket::ListenSocket(const char* port) : Socket(port)
    {
//        std::cout<<fd_max << std::endl;
//        this->fd_max = Socket::fd_max;
//        this->master = Socket::master;
//        base = new SUBD();
        while(1) {
            read_fds = master; // копируем его
            if (select(fd_max + 1, &read_fds, NULL, NULL, NULL) == -1) {
                throw std::runtime_error("Select error");
            }
            check_connections();
        }
    }

    char* ListenSocket::recieve_ip(struct sockaddr_storage &remoteaddr)
    {
        return (inet_ntoa(get_in_addr((struct sockaddr*)&remoteaddr)));
    }

    in_addr &ListenSocket::get_in_addr(struct sockaddr *sa)
    {
        return ((struct sockaddr_in*)sa)->sin_addr;
    }

    void ListenSocket::new_client()
    {
		char *ipv4;
		socklen_t addrlen;
        struct sockaddr_storage remoteaddr;

        addrlen = sizeof remoteaddr;
        int fd_new = accept(listener,
                            (struct sockaddr *)&remoteaddr,
                            &addrlen);
        if (fd_new == -1) {
            std::cerr << "Error Accept" << std::endl;
        }
        else
        {
//			std::cout << "Debug" << std::endl;
            FD_SET(fd_new, &master); // добавляем в мастер-сет
            if (fd_new > fd_max) {    // продолжаем отслеживать самый большой номер дескиптора
                fd_max = fd_new;
            }
            ipv4 = recieve_ip(remoteaddr);
            std::cout   <<  "New connection from "
                        <<  ipv4
                        <<  " on socket "
                        <<  fd_new
                        <<  std::endl;
            this->clients.push_back(Client(fd_new));

//			base->cl.ipv4 = ipv4;
//            base->cl.fds = fd_new;
//            base->cl.nick = "";
//            base->cl.password = "";
//            base->cl.user = "";
//            base->val = std::make_pair(fd_new, base->cl);
//            base->base_fd.insert(base->val);
//            base->base_fd[4].fds = 45;
//            base->base_nick.insert(base->val);
        }
    }

    void ListenSocket::handle_chat(int &i)
    {
		char buf[BUFFER_SIZE];
        int nbytes;
        if ((nbytes = recv(i, buf, sizeof buf, 0)) <= 0)
        {// получена ошибка или соединение закрыто клиентом
            if (nbytes == 0) {
                // соединение закрыто
                printf("selectserver: socket %d hung up\n", i);
            }
            else
            {
                std::cerr << "Error recieve" << std::endl;
            }
            close(i); // bye!
            FD_CLR(i, &master); // удаляем из мастер-сета
        }
        else
        {// у нас есть какие-то данные от клиента
			// если fd нет, UB
			

            // if (handle_message(buf, client) == 1)
            // {
                Parser pars(clients, buf, i);
                // for(int j = 0; j <= fd_max; j++)
                // {
                //     // отсылаем данные всем!
                //     if (FD_ISSET(j, &master))
                //     {// кроме слушающего сокета и клиента, от которого данные пришли
                //         if (j != listener && j != i)
                //         {
                //             if (send(j, buf, nbytes, 0) == -1)
                //             {
                //                 std::cerr << "Error send" << std::endl;
                //             }
                //         }
                //     }
                // }
            // }
        }
    }

	int ListenSocket::handle_message(const char *buf, Client *client) {
		if (client->getNick().empty()) {
			client->setNick(buf);
		} else {
			Client *to = std::find_if(clients.begin(), clients.end(), is_nickname(buf)).base();
			if (to != clients.end().base()) {
				if (send(to->getFd(), "ping\n", 5, 0) < 0) {
					std::cerr << "Error send" << std::endl;
				}
			}
		}
		return 1;
	}

	void ListenSocket::check_connections() {
        socklen_t addrlen;
        struct sockaddr_storage remoteaddr;

//        int listener = Socket::getListener();
        // проходим через существующие соединения, ищем данные для чтения
        for(int i = 0; i <= fd_max; i++) {
            if (FD_ISSET(i, &read_fds))
            { // есть!
                if (listener == i)
                {// обрабатываем новые соединения
                    new_client();
                }
                else
                {// обрабатываем данные клиента
                   handle_chat(i);
                } // Закончили обрабатывать данные от клиента
            } // Закончили обрабатывать новое входящее соединение
        } // Закончили цикл по дескрипторам
    }

    // ListenSocket::SUBD::SUBD() 
    // {
    //     cl = new client();
    // }

    // ListenSocket::SUBD::~SUBD() 
    // {
    //     delete cl;
    // }

    ListenSocket::~ListenSocket()
    {
//        delete base;
    }
}
//}