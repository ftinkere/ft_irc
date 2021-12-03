#include <fstream>
#include <sstream>
#include "ListenSocket.hpp"
#include <set>
#include <algorithm>
#include <fcntl.h>

namespace IRC {

	static std::map<int, std::string> buffers;

	std::map<std::string, std::string> get_config(const std::string &filepath) {
		std::string buf;
		std::fstream conf;
		conf.open(filepath.c_str());
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

	ListenSocket::ListenSocket(const char *port) : Socket(port), read_fds(master) {
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
		while (true) {
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
			this->servername = configs.find("servername")->second;
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

	char *ListenSocket::recieve_ip(struct sockaddr_storage &remoteaddr) {
		return (inet_ntoa(get_in_addr((struct sockaddr *) &remoteaddr)));
	}

	in_addr &ListenSocket::get_in_addr(struct sockaddr *sa) {
		return ((struct sockaddr_in *) sa)->sin_addr;
	}

	void ListenSocket::new_client() {
		char *ipv4;
		struct sockaddr_storage remoteaddr = {0};
		socklen_t addrlen = sizeof remoteaddr;

		int fd_new = accept(listener,
							reinterpret_cast<sockaddr *>(&remoteaddr),
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
			this->clients.push_back(Client(fd_new));

			struct sockaddr_in sa = {0};
			inet_pton(AF_INET, ipv4, &sa.sin_addr); // check
			sa.sin_family = AF_INET;

			char node[NI_MAXHOST];
			memset(node, 0, NI_MAXHOST);
			int res = getnameinfo(reinterpret_cast<const sockaddr *>(&sa), sizeof(sa), node, sizeof(node), NULL, 0, 0);
			// TODO: errs

			if (res == 0) {
				this->clients.back().host = node;
			} else {
				this->clients.back().host = ipv4;
			}
			this->clients.back().login_time = time(NULL);
			std::cout << "New connection from "
					  << ipv4 << " : " << this->clients.back().host
					  << " on fd " << fd_new
					  << std::endl;
		}
	}

	void ListenSocket::handle_chat(int const &fd, std::vector<int> &to_del) {
		char buffer[BUFFER_SIZE] = {0};
		ssize_t bytesRead;

		if ((bytesRead = recv(fd, buffer, BUFFER_SIZE - 1, MSG_PEEK)) <= 0) { // получена ошибка или соединение закрыто клиентом
			if (bytesRead == 0) {
				// соединение закрыто
				std::cout << "fd " << fd << " hung up" << std::endl;
			} else {
				std::cerr << "Error recieve on fd " << fd << std::endl;
				std::cerr << "\t" << errno << ": " << strerror(errno) << std::endl;
			}
//			quit_client(fd);
			to_del.push_back(fd);
		} else { // у нас есть какие-то данные от клиента

			while ((bytesRead = recv(fd, buffer, BUFFER_SIZE - 1, 0)) > 0) {
				buffer[bytesRead] = 0;
				if (buffers.find(fd) == buffers.end()) {
					buffers.insert(std::make_pair(fd, ""));
				}
				buffers.at(fd) += std::string(buffer);
				buffer[0] = 0;
				if (buffers[fd].find("\n") != std::string::npos) {
					break;
				}
			}
			if (bytesRead < 0) {
				std::cerr << "Error recieve on fd " << fd << std::endl;
				std::cerr << "\t" << errno << ": " << strerror(errno) << std::endl;
				return;
			}
			while (buffers[fd].find("\n\r") != std::string::npos) {
				buffers.at(fd).replace(buffers[fd].find("\n\r"), 2, "\n");
			}

			std::cout << "[DEBUG]: buffer: " << std::endl
			<< buffers[fd] << " EOF" << std::endl;

			std::vector<std::string> msgs = split(buffers[fd], '\n');

			if (!buffers[fd].empty() && buffers[fd][buffers[fd].size() - 1] != '\n') {
				buffers.at(fd) = msgs.back();
				msgs.pop_back();
			} else {
				buffers.at(fd).clear();
			}

			std::list<Client>::iterator client = std::find_if(clients.begin(), clients.end(), is_fd(fd));
			for (std::vector<std::string>::iterator it = msgs.begin(); it != msgs.end(); ++it) {
				std::cout << *it << " ";
				if (!it->empty()) {
					std::cout << "[DEBUG]: before parse: " << *it << std::endl;
					Command cmd(*it);
					cmd.exec(*client, *this);
				}
			}
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
//		socklen_t addrlen;addrlen
//		struct sockaddr_storage remoteaddr;

		if (FD_ISSET(listener, &read_fds)) {// обрабатываем новые соединения
			new_client();
		}

		std::vector<int> to_del;
		for (std::list<Client>::iterator it = clients.begin(); it != clients.end(); ++it) {
			// Таймаутит незарегестрированных пользователей
			if (!it->isFlag(UMODE_REGISTERED)
				&& it->login_time + this->registration_timeout - time(NULL) <= 0) {
				// reply timeout
				{
					Command cmd("", "ERROR");
					cmd << "Registration timeout";
					send_command(cmd, it->getFd());
				}
				to_del.push_back(it->getFd());
//				quit_client(it->getFd());
//				break;
			} else if (FD_ISSET(it->getFd(), &read_fds)) {
				handle_chat(it->getFd(), to_del);
			}
		}

		for (std::vector<int>::reverse_iterator it = to_del.rbegin(); it != to_del.rend(); ++it) {
			quit_client(*it);
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
			Command quit(cl->get_full_name(), CMD_QUIT);
			for (std::set<Client *>::iterator itc = channels[*it].users.begin();
				 itc != channels[*it].users.end(); ++itc) {
				this->send_command(quit, (*itc)->getFd());
			}
		}
		std::cout << "[DEBUG]: " << cl->get_full_name() << " (" << fd << ") quit" << std::endl;
		clients.erase(cl);
		close(fd); // bye!
		FD_CLR(fd, &master); // удаляем из мастер-сета
		buffers.erase(fd);
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
			sendError(feedback, *this, ERR_NOSUCHNICK, nick, "");
			return NULL;
		}
		if (feedback.getFlags() & UMODE_OPER) {
			return &(*it);
		} else {
			sendError(feedback, *this, ERR_USERSDONTMATCH, "",
					  "");//если не опер то не можешь редактировать чужой ник
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

