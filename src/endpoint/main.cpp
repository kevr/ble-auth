#include "socket.h"
#include <iostream>
using namespace std;

int main(int argc, char *argv[])
{
	if(argc < 2) {
		std::cerr << "usage: " << argv[0] << " host [port]" << std::endl;
		return 1;
	}

	std::string host(argv[1]);
	unsigned short port = 6666;
	if(argc > 2) {
		try {
			port = std::stoi(argv[2]);
		} catch(...) {
			std::cerr << "Port argument must be a number 1-65535" << std::endl;
			return 2;
		}
	}

	boost::asio::io_service io;
	net::socket sock(io);

	sock.on_read([](net::socket& sock, std::string str) {
		if(str == "lock") {
			system("gnome-screensaver-command -l");
		} else if(str == "unlock") {
			system("gnome-screensaver-command -d && xset s reset");
		}
	});

	sock.run(host, port);
	io.run();
	return 0;
}

