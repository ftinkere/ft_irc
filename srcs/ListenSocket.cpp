#include <fstream>
#include <sstream>
#include "ListenSocket.hpp"
#include <set>

namespace IRC {

	std::map<std::string, std::string> get_config(std::string filepath) {
		std::string buf;
		std::fstream conf;
//		filepath = "feefef";
		conf.open(filepath);
		std::map<std::string, std::string> confs;
		if (!conf.is_open()) {
			return confs;
		}

		while (!conf.eof()) {
			std::getline(conf, buf);
			std::size_t pos = buf.find('=');
			if (pos != std::string::npos) {
				std::string s1 = buf.substr(0, pos);
				std::string s2 = buf.substr(pos + 1);
				confs.insert(std::pair<std::string, std::string>(s1, s2));
			}
		}
		return (confs);
	}

	ListenSocket::ListenSocket(const char *port) : Socket(port), servername(""), password("") {
		commands[CMD_PASS] = &cmd_pass;
		commands[CMD_NICK] = &cmd_nick;
		commands[CMD_USER] = &cmd_user;
		commands[CMD_QUIT] = &cmd_quit;
		commands[CMD_PRIVMSG] = &cmd_privmsg;
		commands[CMD_NOTICE] = &cmd_notice;
		commands[CMD_AWAY] = &cmd_away;
		commands[CMD_JOIN] = &cmd_join;
		commands[CMD_PART] = &cmd_part;
		commands[CMD_TOPIC] = &cmd_topic;
		commands[CMD_NAMES] = &cmd_names;
		commands[CMD_LIST] = &cmd_list;
		commands[CMD_INVITE] = &cmd_invite;
		commands[CMD_KICK] = &cmd_kick;
		commands[CMD_MODE] = &cmd_mode;
		commands[CMD_OPER] = &cmd_oper;
		commands[CMD_KILL] = &cmd_kill;
		commands[CMD_ADMIN] = &cmd_admin;
		commands[CMD_WHOIS] = &cmd_whois;
		Channel::modes.insert(std::make_pair(TOPIC, Channel::T));
		Channel::modes.insert(std::make_pair(INVIT, Channel::I));
		Channel::modes.insert(std::make_pair(MODES, Channel::M));
		Channel::modes.insert(std::make_pair(SECRET, Channel::S));
		Channel::modes.insert(std::make_pair(SPEAK, Channel::N)); //это все для более быстрого поиска модов
		Channel::modes.insert(std::make_pair(OPER, Channel::O));
		Channel::modes.insert(std::make_pair(VOICER, Channel::V));
		Channel::modes.insert(std::make_pair(KEY, Channel::K));
		Channel::modes.insert(std::make_pair(LEN, Channel::L));
		//frfgrg
	}

	void ListenSocket::execute() {
//        std::cout<<fd_max << std::endl;
//        this->fd_max = Socket::fd_max;
//        this->master = Socket::master;
//        base = new SUBD();
		while (1) {
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

	void ListenSocket::configure(std::string const &path) {
		std::map<std::string, std::string> configs = get_config(path);
		if (configs.find("servername") != configs.end()) {
			this->servername = configs["servername"];
		} else {
			char name[1024];
			name[1023] = '\0';
			gethostname(name, 1023);
			this->servername = name;
		}
		std::map<std::string, std::string>::iterator it = configs.find("operators");
		if (it != configs.end()) {
			if (it->second == "yes") {
				opers = get_config("operators.conf");
			}
		}
		admin.insert(std::make_pair("adminName", configs["adminName"]));
		admin.insert(std::make_pair("adminNickname", configs["adminNickname"]));
		admin.insert(std::make_pair("adminEmail", configs["adminEmail"]));
		std::cout << "[DEBUG]: servername set to " << this->servername << std::endl;
	}

//	std::vector<Client*> ListenSocket::find_clients(std::string const& nick, int flag, Client const& )
//	{
//		std::vector<Client*> collection;
//		if (nick[0] == '#')
//		{
//			//отправляем сообщение всем пользователям канала
//			//if flag != -1
//			//если нет прав или нет членства ERR_CANNOTSENDTOCHAN
//		}
//		else if (nick[0] == '@' && nick[1] == '#')
//		{
//			//отправляем сообщение всем админам канала
//			//if flag != -1
//			//если нет прав или нет членства ERR_CANNOTSENDTOCsHAN
//		}
//		else
//		{
//			//отправляем сообщение конкретному никнейму
//            Client *to = std::find_if(clients.begin(), clients.end(), is_nickname(nick)).base();
//            if (to == clients.end().base())
//			{
//				if (flag != -1)
//					sendError(client, *this, ERR_NOSUCHNICK, nick, "");
//				return collection;
////                reinterpret_cast<const Client *&>(to)
//			}
//			collection.reserve(1);
//			collection.push_back(to);
//			return collection;
//		}
//        return collection;
//	}

	char *ListenSocket::recieve_ip(struct sockaddr_storage &remoteaddr) {
		return (inet_ntoa(get_in_addr((struct sockaddr *) &remoteaddr)));
	}

	in_addr &ListenSocket::get_in_addr(struct sockaddr *sa) {
		return ((struct sockaddr_in *) sa)->sin_addr;
	}

	void ListenSocket::new_client() {
		char *ipv4;
		socklen_t addrlen;
		struct sockaddr_storage remoteaddr;

		addrlen = sizeof remoteaddr;
		int fd_new = accept(listener,
							(struct sockaddr *) &remoteaddr,
							&addrlen);
		if (fd_new == -1) {
			std::cerr << "Error Accept" << std::endl;
		} else {
//			std::cout << "Debug" << std::endl;
			FD_SET(fd_new, &master); // добавляем в мастер-сет
			if (fd_new > fd_max) {    // продолжаем отслеживать самый большой номер дескиптора
				fd_max = fd_new;
			}
			ipv4 = recieve_ip(remoteaddr);
			std::cout << "New connection from "
					  << ipv4
					  << " on socket "
					  << fd_new
					  << std::endl;
			this->clients.push_back(Client(fd_new));

			struct sockaddr_in sa;
			inet_pton(AF_INET, ipv4, &sa.sin_addr); // check
			sa.sin_family = AF_INET;

			char node[NI_MAXHOST];
			memset(node, 0, NI_MAXHOST);
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

	void ListenSocket::handle_chat(int const &i) {
		char buf[BUFFER_SIZE];
		int nbytes;
		if ((nbytes = recv(i, buf, sizeof buf - 1, 0)) <= 0) {// получена ошибка или соединение закрыто клиентом
			if (nbytes == 0) {
				// соединение закрыто
				printf("selectserver: socket %d hung up\n", i);
			} else {
				std::cerr << "Error recieve" << std::endl;
			}
//			clients.erase(std::find_if(clients.begin(), clients.end(), is_fd(i)));
//            close(i); // bye!
//            FD_CLR(i, &master); // удаляем из мастер-сета
			quit_client(i);
		} else {// у нас есть какие-то данные от клиента
			// если fd нет, UB
			buf[nbytes] = '\0';
			std::list<Client>::iterator client = std::find_if(clients.begin(), clients.end(), is_fd(i));
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
			std::list<Client>::iterator to = std::find_if(clients.begin(), clients.end(), is_nickname(buf));
			if (to != clients.end()) {
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

		if (FD_ISSET(listener, &read_fds)) {// обрабатываем новые соединения
			new_client();
		}

		for (std::list<Client>::iterator it = clients.begin(); it != clients.end(); ++it) {
			// Таймаутит незарегестрированных пользователей
			if (!(it->getFlags() & UMODE_REGISTERED)
				&& it->login_time + this->registration_timeout - time(NULL) <= 0) {
				// reply timeout
				{
					Command cmd("ERROR");
					cmd << "Registration timeout";
					send_command(cmd, it->getFd());
				}
				quit_client(it->getFd());
				break;
			}
			if (FD_ISSET(it->getFd(), &read_fds)) {
				handle_chat(it->getFd());
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

	ListenSocket::~ListenSocket() {
//        delete base;
	}

	void ListenSocket::set_password(const std::string &password) {
		this->password = password;
	}

	void ListenSocket::quit_client(int fd) {
		std::list<Client>::iterator cl = std::find_if(clients.begin(), clients.end(), is_fd(fd));
		for (std::list<std::string>::iterator it = cl->getChannels().begin();
			 it != cl->getChannels().end(); ++it) {
			channels[*it].erase_client(*cl); //удаляем из всех групп
		}
		clients.erase(cl);
		close(fd); // bye!
		FD_CLR(fd, &master); // удаляем из мастер-сета
	}

	const std::list<Client> &ListenSocket::getClients() const { return clients; }

	const fd_set &ListenSocket::getReadFds() const { return read_fds; }

	const std::string &ListenSocket::getServername() const { return servername; }

	const std::string &ListenSocket::getPassword() const { return password; }

	const std::map<std::string, ListenSocket::cmd> &ListenSocket::getCommands() const { return commands; }

	std::vector<Client *> IRC::ListenSocket::find_clients(const std::string &nick, int flag, Client const &feedback) {
		std::vector<Client *> ret;

		if (nick[0] == '#') {
			// find in channel
			if (channels.find(nick) != channels.end()) {
				Channel &chan = channels[nick];
				if (chan.isFlag(CMODE_NOEXT) || chan.users.find(&const_cast<Client &>(feedback)) !=
												chan.users.end()) { //если есть +n или клиент состоит в группе
					if (chan.isFlag(CMODE_MODER) || chan.voiced.find(&(feedback.getNick())) != chan.voiced.end()) {
						for (std::set<Client *>::iterator it = chan.users.begin(); it != chan.users.end(); ++it) {
//							Client *client = *it;
							ret.push_back(*it);
						}
					} else if (flag != -1) {
						sendError(feedback, *this, ERR_CANNOTSENDTOCHAN, nick, "");
					}
				} else if (flag != -1) {
					sendError(feedback, *this, ERR_CANNOTSENDTOCHAN, nick, "");
				}
			} else if (flag != -1) {
					sendError(feedback, *this, ERR_NOSUCHCHANNEL, nick, "");
				}
				//обработать +m
		} else if (nick.size() > 2 && nick[0] == '@' && nick[1] == '#') {
			// find opers in channel
			std::string nick1 = nick.substr(1);
			if (channels.find(nick1) != channels.end()) {
				Channel &chan = channels[nick1];
				if (chan.isFlag(CMODE_NOEXT) || (chan.users.find(&const_cast<Client &>(feedback)) != chan.users.end()))
					//если есть +n или клиент состоит в группе
				{
					if (chan.isFlag(CMODE_MODER) || chan.voiced.find(&(feedback.getNick())) != chan.voiced.end()) {
						std::set<std::string const *>::iterator it = chan.opers.begin();//берем всех оперов
						for (; it != chan.opers.end(); ++it) {
							std::list<Client>::iterator to = std::find_if(clients.begin(),
																		  clients.end(),
																		  is_nickname(**it));
							ret.push_back(&(*to));
						}
					} else {
						if (flag != -1)
							sendError(feedback, *this, ERR_CANNOTSENDTOCHAN, nick1, "");
					}
				} else {
					if (flag != -1)
						sendError(feedback, *this, ERR_CANNOTSENDTOCHAN, nick1, "");
				}
			} else if (flag != -1) {
				sendError(feedback, *this, ERR_NOSUCHCHANNEL, nick1, "");
			}
		} else {
			// find nick
			std::list<Client>::iterator it = std::find_if(this->clients.begin(), this->clients.end(),
														  is_nickname(nick));
			if (it == this->clients.end()) {
				if (flag != -1) {
					sendError(feedback, *this, ERR_NOSUCHNICK, nick, "");
				}
			} else {
				ret.push_back(&(*it));
			}
		}
		return ret;
	}

	std::vector<Client *> ListenSocket::find_clients(const std::string &nick, const Client &feedback) {
//		std::vector<Client*> IRC::ListenSocket::find_clients(const std::string &nick, Client const& feedback) {
//		std::vector<Client *> IRC::ListenSocket::find_clients(const std::string &nick, Client const &feedback) {
		return find_clients(nick, 0, feedback);
	}


	Client *IRC::ListenSocket::thisisnick(const std::string &nick, int flag, Client &feedback) {
		if (feedback.getNick() == nick)
			return &feedback;
		std::list<Client>::iterator it = std::find_if(this->clients.begin(), this->clients.end(),
													  is_nickname(nick));
		if (it == this->clients.end()) {
			sendError(feedback, *this, ERR_NOSUCHNICK, nick);
			return NULL;
		}
		if (feedback.getFlags() & UMODE_NOPER) {
			return &(*it);
		} else {
			if ((*it).isFlag(UMODE_INVIS)){
				sendError(feedback, *this, ERR_NOSUCHNICK, nick);
				return NULL;
			}
			sendError(feedback, *this, ERR_USERSDONTMATCH);//если не опер то не можешь редактировать чужой ник
			return NULL;
		}
	}

	Channel *IRC::ListenSocket::thisischannel(const std::string &nick, int flag, Client &feedback) {
		std::map<std::string, Channel>::iterator it = channels.find(nick);
		if (it == channels.end()) {
			sendError(feedback, *this, ERR_NOSUCHCHANNEL, nick, "");
			return (NULL);
		}
		return &(it->second);
	}

	void ListenSocket::send_command(const Command &command, const Client &client) {
		send_command(command, client.getFd());
	}

	void ListenSocket::send_command(const Command &command, const std::string &nickname) {
		std::list<Client>::const_iterator to = std::find_if(getClients().begin(), getClients().end(),
															is_nickname(nickname));
		if (to != getClients().end()) {
			send_command(command, to->getFd());
		}
	}

	void ListenSocket::send_command(const Command &command, int fd) {
		std::string message = command.to_string();
		send(fd, message.c_str(), message.size(), 0);
	}


}

