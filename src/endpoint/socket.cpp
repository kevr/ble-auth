#include "socket.h"

void net::default_on_read(net::socket& sock, std::string data)
{
	std::cout << "read: " << data << std::endl;
}

