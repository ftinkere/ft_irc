#include "Socket.hpp"
#include "ListeningSocket.hpp"

#define PORT "9034"   // порт, который мы слушаем


int main(void)
{
    IRC::Socket *sock = new IRC::ListenSocket(PORT);
    delete sock;
}