

#include "ListeningSocket.hpp"

#define PORT "9034"   // порт, который мы слушаем




int main(void)
{
	IRC::ListenSocket server(PORT);
	server.configure("config.conf");
	server.execute();
}