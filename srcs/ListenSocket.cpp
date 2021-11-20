#include <fstream>
#include <sstream>
#include <Parser.hpp>
#include "ListenSocket.hpp"
#include "Command.hpp"
#include "commands.hpp"

namespace IRC{

	std::map<std::string, std::string> get_config(std::string filepath)
	{
		std::string buf;
		std::fstream conf;
//		filepath = "feefef";
		conf.open(filepath);
		std::map<std::string, std::string> confs;
		if (!conf.is_open()) {
			return confs;
		}

		while (!conf.eof())
		{
			std::getline(conf, buf);
			std::size_t pos = buf.find('=');
			if (pos != std::string::npos)
			{
				std::string s1 = buf.substr(0, pos);
				std::string s2 = buf.substr(pos + 1);
				confs.insert(std::pair<std::string, std::string>(s1, s2));
			}
		}
		return (confs);
	}

	ListenSocket::ListenSocket(const char* port) : Socket(port), servername(""), password("") {
		commands[CMD_PASS] = &cmd_pass;
		commands[CMD_NICK] = &cmd_nick;
		commands[CMD_USER] = &cmd_user;
	}

	void ListenSocket::execute() {
//        std::cout<<fd_max << std::endl;
//        this->fd_max = Socket::fd_max;
//        this->master = Socket::master;
//        base = new SUBD();
		while(1) {
			read_fds = master; // копируем его
			struct timeval timeout = {
					.tv_sec = 1,
					.tv_usec = 0
			};
			if (select(fd_max + 1, &read_fds, NULL, NULL, &timeout) == -1) {
				throw std::runtime_error("Select error");
			}
			check_connections();
		}
	}

	void ListenSocket::configure(std::string const& path) {
		std::map<std::string, std::string> configs = get_config(path);
		if (configs.find("servername") != configs.end()) {
			this->servername = configs["servername"];
		} else {
			char name [1024];
			name[1023] = '\0';
			gethostname(name, 1023);
			this->servername = name;
		}
		std::cout << "[DEBUG]: servername set to " << this->servername << std::endl;
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

			struct sockaddr_in sa;
			inet_pton(AF_INET, ipv4, &sa.sin_addr); // check
			sa.sin_family = AF_INET;

			char node[NI_MAXHOST];
			memset(node,0, NI_MAXHOST);
			int res = getnameinfo(reinterpret_cast<const sockaddr *>(&sa), sizeof(sa), node, sizeof(node), NULL, 0, 0);
			// TODO: errs
//			std::cout << "[DEBUG]: " << node << std::endl;

			if (res == 0) {
				this->clients.back().host = node;
			} else {
				this->clients.back().host = ipv4;
			}
			this->clients.back().login_time = time(NULL);

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

    void ListenSocket::handle_chat(int const& i)
    {
		char buf[BUFFER_SIZE];
        int nbytes;
        if ((nbytes = recv(i, buf, sizeof buf - 1, 0)) <= 0)
        {// получена ошибка или соединение закрыто клиентом
            if (nbytes == 0) {
                // соединение закрыто
                printf("selectserver: socket %d hung up\n", i);
            }
            else
            {
                std::cerr << "Error recieve" << std::endl;
            }
//			clients.erase(std::find_if(clients.begin(), clients.end(), is_fd(i)));
//            close(i); // bye!
//            FD_CLR(i, &master); // удаляем из мастер-сета
			quit_client(i);
        }
        else
        {// у нас есть какие-то данные от клиента
			// если fd нет, UB
			buf[nbytes] = '\0';
			Client* client = std::find_if(clients.begin(), clients.end(), is_fd(i)).base();
			Command cmd(buf);
			cmd.exec(*client, *this);
//			cmd.send_to(*client, *this);
//            if (handle_message(buf, client) == 1){
//                for(int j = 0; j <= fd_max; j++) {
//                    // отсылаем данные всем!
//                    if (FD_ISSET(j, &master))
//                    {// кроме слушающего сокета и клиента, от которого данные пришли
//                        if (j != listener && j != i)
//                        {
//                            if (send(j, buf, nbytes, 0) == -1)
//                            {
//                                std::cerr << "Error send_to" << std::endl;
//                            }
//                        }
//                    }
//                }
//            }
        }
    }

	int ListenSocket::handle_message(const char *buf, Client *client) {
		if (client->getNick().empty()) {
			client->setNick(buf);
		} else {
			Client *to = std::find_if(clients.begin(), clients.end(), is_nickname(buf)).base();
			if (to != clients.end().base()) {
				if (send(to->getFd(), "ping\n", 5, 0) < 0) {
					std::cerr << "Error send_to" << std::endl;
				}
			}
		}
		return 1;
	}

	void ListenSocket::check_connections() {
        socklen_t addrlen;
        struct sockaddr_storage remoteaddr;

		if (FD_ISSET(listener, &read_fds))
		{// обрабатываем новые соединения
			new_client();
		}

		for (int i = 0; i < clients.size(); ++i) {
			// Таймаутит незарегестрированных пользователей
			if (!(clients[i].getFlags() & UMODE_REGISTERED)
					&& clients[i].login_time + this->registration_timeout - time(NULL) <= 0) {
				// reply timeout
			{
				std::vector<std::string> params;
				params.push_back("Registration timeout");
				Command("", "ERROR", params).send_to(clients[i], *this);
			}
				quit_client(clients[i].getFd());
				break;
			}
			if (FD_ISSET(clients[i].getFd(), &read_fds)) {
				handle_chat(clients[i].getFd());
			}
		}

//        int listener = Socket::getListener();
        // проходим через существующие соединения, ищем данные для чтения
//        for(int i = 0; i <= fd_max; i++) {
//            if (FD_ISSET(i, &read_fds))
//            { // есть!
//                if (listener == i)
//                {// обрабатываем новые соединения
//                    new_client();
//                }
//                else
//                {// обрабатываем данные клиента
//                   handle_chat(i);
//                } // Закончили обрабатывать данные от клиента
//            } // Закончили обрабатывать новое входящее соединение
//        } // Закончили цикл по дескрипторам
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

	void ListenSocket::set_password(const std::string &password) {
		this->password = password;
	}

	void ListenSocket::quit_client(int fd) {
		clients.erase(std::find_if(clients.begin(), clients.end(), is_fd(fd)));
		close(fd); // bye!
		FD_CLR(fd, &master); // удаляем из мастер-сета
	}

	const std::vector<Client> &ListenSocket::getClients() const { return clients; }
	const fd_set &ListenSocket::getReadFds() const { return read_fds; }
	const std::string &ListenSocket::getServername() const { return servername; }
	const std::string &ListenSocket::getPassword() const { return password; }
	const std::map<std::string, ListenSocket::cmd> &ListenSocket::getCommands() const { return commands; }
}
//}