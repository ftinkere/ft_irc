#include "types.hpp"
#include "ListenSocket.hpp"
#include <Parser.hpp>
#include <Reply.hpp>
#include <algorithm>
#include <fcntl.h>
#include <fstream>
#include <set>
#include <sstream>
#include <csignal>

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

	ListenSocket::ListenSocket(const char *port) : Socket(port), read_fds(master), creation_time(time(NULL)) {
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
		commands[CMD_PING] = &cmd_ping;
		commands[CMD_PONG] = &cmd_pong;
		commands[CMD_WHOIS] = &cmd_whois;
		commands[CMD_WALLOPS] = &cmd_wallops;
		commands[CMD_WHOWAS] = &cmd_whowas;

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

	bool is_work = true;

	void termination_handler (int signum) {
		(void)signum;
		is_work = false;
	}

	void ListenSocket::execute() {
//        std::cout<<fd_max << std::endl;
//        this->fd_max = Socket::fd_max;
//        this->master = Socket::master;
//        base = new SUBD();
		signal(SIGINT, termination_handler);
		signal(SIGTERM, termination_handler);
		while (is_work) {
			read_fds = master; // копируем его
			struct timespec timeout_p = {
					.tv_sec = 1,
					.tv_nsec = 0
			};
			sigset_t sigs;
			sigemptyset(&sigs);
			sigaddset(&sigs, SIGINT);
			sigaddset(&sigs, SIGTERM);
			if (pselect(fd_max + 1, &read_fds, NULL, NULL, &timeout_p, &sigs) == -1) {
				std::cout << "Select error " << errno << ": " << strerror(errno) << std::endl;
				is_work = false;
				throw std::runtime_error("Select error");
			} else {
				check_connections();
			}
//			break;
		}
		std::cout << "Server quit" << std::endl;
		signal(SIGINT, SIG_DFL);
		signal(SIGTERM, SIG_DFL);
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
		struct sockaddr_storage remoteaddr;
		socklen_t addrlen = sizeof remoteaddr;

		memset(&remoteaddr, 0, sizeof remoteaddr);
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

			struct sockaddr_in sa;
			memset(&sa, 0, sizeof sa);
			inet_pton(AF_INET, ipv4, &sa.sin_addr); // check
			sa.sin_family = AF_INET;

			char node[NI_MAXHOST];
			memset(node, 0, NI_MAXHOST);
			int res = getnameinfo(reinterpret_cast<const sockaddr *>(&sa), sizeof(sa), node, sizeof(node), NULL, 0, 0);

			if (res == 0) {
				this->clients.back().setHost(node);
			} else {
				this->clients.back().setHost(ipv4);
			}
			this->clients.back().touchRegisterTime();
			this->clients.back().touchPingpongTime();
			std::cout << "New connection from "
					  << ipv4 << " : " << this->clients.back().getHost()
					  << " on fd " << fd_new
					  << std::endl;
		}
	}

	void ListenSocket::handle_chat(int const &fd) {
		char buffer[BUFFER_SIZE] = {0};
		ssize_t bytesRead;

		client_iter client = getClient(fd);

		if ((bytesRead = recv(fd, buffer, BUFFER_SIZE - 1, MSG_PEEK)) <=
			0) { // получена ошибка или соединение закрыто клиентом
			if (bytesRead == 0) {
				// соединение закрыто
				std::cout << "fd " << fd << " hung up" << std::endl;
			} else {
				std::cerr << "Error recieve on fd " << fd << std::endl;
				std::cerr << "\t" << errno << ": " << strerror(errno) << std::endl;
			}
			client->disconnect();
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
			while (buffers[fd].find("\r\n") != std::string::npos) {
				buffers.at(fd).replace(buffers[fd].find("\r\n"), 2, "\n");
			}

//			std::cout << "[DEBUG]: buffer: " << std::endl
//			<< buffers[fd] << " EOF" << std::endl;

			std::vector<std::string> msgs = split(buffers[fd], '\n');

			if (!buffers[fd].empty() && buffers[fd][buffers[fd].size() - 1] != '\n') {
				buffers.at(fd) = msgs.back();
				msgs.pop_back();
			} else {
				buffers.at(fd).clear();
			}

			for (std::vector<std::string>::iterator it = msgs.begin(); it != msgs.end(); ++it) {
				if (!it->empty()) {
					if (!client->isPinged()) {
						client->unsetPinged();
						client->touchPingpongTime();
					}
					std::cout << "[DEBUG]: cmd: " << *it << std::endl;
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
			client_iter to = std::find_if(clients.begin(), clients.end(), is_nickname(buf));
			if (to != clients.end()) {
				if (send(to->getFd(), "ping\n", 5, 0) < 0) {
					std::cerr << "Error send_to" << std::endl;
				}
			}
		}
		return 1;
	}

	void ListenSocket::check_connections() {
		std::vector<int> to_del;

		if (FD_ISSET(listener, &read_fds)) {
			// обрабатываем новые соединения
			new_client();
		}

		for (client_iter it = clients.begin(); it != clients.end(); ++it) {
			if (it->isDisconect()) {
				to_del.push_back(it->getFd());
				continue;
			} else if (!it->isFlag(UMODE_REGISTERED)
				&& it->getRegisterTime() + this->registration_timeout <= time(NULL)) {
				// Таймаутит незарегестрированных пользователей
//				send_command(Command("", "ERROR", "Registration timeout"), *it);
				std::stringstream ss;
				ss << this->registration_timeout;
				it->disconnect("Registration timeout: " + ss.str() + " seconds");
				continue;
			} else if (it->isFlag(UMODE_REGISTERED)
					   && !it->isPinged() && it->getPingpongTime() + this->ping_period <= time(NULL)) {
				send_command(*it, CMD_PING, it->getNick());
				it->setPinged();
				it->touchPingpongTime();
			} else if (it->isFlag(UMODE_REGISTERED)
					   && it->isPinged() && it->getPingpongTime() + this->pong_timeout <= time(NULL)) {
				send_command(Command("", "ERROR", "Ping timeout"), *it);
				std::stringstream ss;
				ss << this->pong_timeout;
				it->disconnect("Pong timeout: " + ss.str() + " seconds");
				continue;
			}
			if (FD_ISSET(it->getFd(), &read_fds)) {
				handle_chat(it->getFd());
			}
		}

		for (std::vector<int>::iterator it = to_del.begin(); it != to_del.end(); ++it) {
			quit_client(*it);
		}

		{
			std::vector<channel_iter> to_del;

			for (channel_iter it = channels.begin(); it != channels.end(); ++it) {
				if (it->second.isDelete()) {
					to_del.push_back(it);
				}
			}
			for (std::vector<channel_iter>::iterator it = to_del.begin(); it != to_del.end(); ++it) {
				channels.erase(*it);
			}
		}

	}

	ListenSocket::~ListenSocket() {
		for (client_reverse_iter it = clients.rbegin(); it != clients.rend(); ++it) {
			close(it->getFd());
		}
	}

	void ListenSocket::set_password(const std::string &password) {
		this->password = password;
	}

	void ListenSocket::quit_client(int fd) {
		client_iter cl = getClient(fd);
		Command quit(cl->get_full_name(), CMD_QUIT, cl->getQuitMsg());
		for (std::list<std::string>::iterator it = cl->getChannels().begin(); it != cl->getChannels().end(); ++it) {
			channels[*it].erase_client(*cl); //удаляем из всех групп
			for (channel_client_iter itc = channels[*it].users.begin();
				 itc != channels[*it].users.end(); ++itc) {
				this->send_command(quit, (*itc)->getFd());
			}
		}
		this->send_command(quit, fd);
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

	const std::map<std::string, cmd> &ListenSocket::getCommands() const { return commands; }


	Client *IRC::ListenSocket::thisisnick(const std::string &nick, int flag, Client &feedback) {
		if (feedback.getNick() == nick)
			return &feedback;
		client_iter it = getClient(nick);
		if (it == this->clients.end()) {
			sendError(feedback, *this, ERR_NOSUCHNICK, nick);
			return NULL;
		}
		if (feedback.isFlag(UMODE_OPER)) {
			return &(*it);
		} else if (flag == 1 && !(*it).isFlag(UMODE_INVIS)) {
			return &(*it);
		} else {
			if ((*it).isFlag(UMODE_INVIS)) {
				sendError(feedback, *this, ERR_NOSUCHNICK, nick);
				return NULL;
			}
			sendError(feedback, *this, ERR_USERSDONTMATCH);//если не опер то не можешь редактировать чужой ник
			return NULL;
		}
	}

	void ListenSocket::send_command(const Command &command, const Client &client) const {
		send_command(command, client.getFd());
	}

	void ListenSocket::send_command(const Command &command, const std::string &nickname) const {
		client_const_iter to = getClient(nickname);
		if (to != getClients().end()) {
			send_command(command, to->getFd());
		}
	}

	void ListenSocket::send_command(const Command &command, int fd) const {
		std::string message = command.to_string();
		send(fd, message.c_str(), message.size(), 0);
	}

	void ListenSocket::send_command(const Client &client, const std::string &cmd, const std::string &arg1,
									const std::string &arg2, const std::string &arg3, const std::string &arg4) const {
		send_command(Command(getServername(), cmd, arg1, arg2, arg3, arg4), client.getFd());
	}

	void ListenSocket::send_command(int fd, const std::string &cmd, const std::string &arg1,
									const std::string &arg2, const std::string &arg3, const std::string &arg4) const {
		send_command(Command(getServername(), cmd, arg1, arg2, arg3, arg4), fd);
	}

	bool ListenSocket::isChannelExist(const std::string &chan) const {
		return channels.find(chan) != channels.end();
	}

	bool ListenSocket::isChannelExist(channel_iter const &chan) const {
		return chan != channels.end();
	}

	bool ListenSocket::isChannelExist(channel_const_iter const &chan) const {
		return chan != channels.end();
	}

	void ListenSocket::addNewChannel(const std::string &chan) {
		if (!isChannelExist(chan)) {
			channels.insert(std::make_pair(chan, Channel(chan)));
		}
	}

	channel_iter ListenSocket::getChannel(const std::string &chan) {
		return channels.find(chan);
	}

	channel_const_iter ListenSocket::getChannel(const std::string &chan) const {
		return channels.find(chan);
	}

	client_iter ListenSocket::getClientByMask(std::string const &mask) {
		return std::find_if(clients.begin(), clients.end(), is_mask(mask));
	}

	client_const_iter ListenSocket::getClientByMask(std::string const &mask) const {
		return std::find_if(clients.begin(), clients.end(), is_mask(mask));
	}

	client_iter ListenSocket::getClient(std::string const &nick) {
		return std::find_if(clients.begin(), clients.end(), is_nickname(nick));
	}

	client_iter ListenSocket::getClient(int fd) {
		return std::find_if(clients.begin(), clients.end(), is_fd(fd));
	}

	client_const_iter ListenSocket::getClient(std::string const &nick) const {
		return std::find_if(clients.begin(), clients.end(), is_nickname(nick));
	}

	client_const_iter ListenSocket::getClient(int fd) const {
		return std::find_if(clients.begin(), clients.end(), is_fd(fd));
	}

	bool ListenSocket::isClientExist(const std::string &nick) const {
		return getClient(nick) != clients.end();
	}

	bool ListenSocket::isClientExist(const client_iter &it) const {
		return it != clients.end();
	}

}

