#include "ListenSocket.hpp"

// TODO: there is a Makefile, the project compiles correctly, is written in C++, the binary file is called ircserv

#define PORT "6667"   // порт, который мы слушаем

int main(int argc, char *argv[])
{

	std::string port = PORT;
	std::string password = "";

	if (argc > 1) {
		port = argv[1];
	}
	if (argc > 2) {
		password = argv[2];
	}

	IRC::ListenSocket server(port.c_str());
	server.configure("config.conf");
	server.set_password(password);
	server.execute();
}