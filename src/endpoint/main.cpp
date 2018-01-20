#include "cache.h"
#include "socket.h"
#include "scanner.h"
#include <iostream>
#include <future>
using namespace std;

int main(int argc, char *argv[])
{
	std::string host;
	if(argc < 2) {
		// Get cache from the system
		auto cache_available = cache::last_host::available();
		std::string cached_host;
		if(cache_available)
			cached_host = cache::last_host::value();

		net::scanner scanner;

		if(cache_available && scanner.scan(cached_host, 32, 6666, host)) {
			std::cout << "discovered ble-server on " << host << std::endl;
		} else if(scanner.scan("192.168.1.0", 24, 6666, host)) {
			std::cout << "found host: " << host << std::endl;
		} else {
			std::cout << "unable to locate host" << std::endl;
			return 1;
		}
	}

	if(argc > 1) {
		host = std::string(argv[1]);
	}

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
	std::unique_ptr<net::socket> sock(new net::socket(io));

	sock->on_read([](net::socket& sock, std::string str) {
		if(str == "lock") {
			system("gnome-screensaver-command -l");
		} else if(str == "unlock") {
			system("gnome-screensaver-command -d && xset s reset");
		}
	});

	int sleep_time = 2;

	while(1) {
		io.reset();
		sock.reset(new net::socket(io));
		// Always keep trying to reconnect
		sock->run(host, port);
		io.run();
		// Sleep for 30 seconds
		std::cout << "disconnected from the server, trying again in "
				  << sleep_time << " seconds" << std::endl;
		// If we just had a valid connection, reset timer
		if(sock->connected()) {
			cache::last_host::set(host);
			sleep_time = 2;
		}

		std::this_thread::sleep_for(std::chrono::seconds(sleep_time));
		if(sleep_time < 32) {
			sleep_time *= 2;
		}
	}

	return 0;
}

