#include "ble.h"
#include "server.h"
#include <vector>
#include <memory>
#include <unordered_map>
#include <fstream>
#include <iostream>
#include <sys/types.h>
#include <pwd.h>
#include <mutex>
#include <thread>

static int sig_received = 0;

void sigint_handler(int sig)
{
	sig_received = sig;
}

struct passwd * get_user(const std::string& user)
{
	auto pwd = getpwnam(user.c_str());
	return pwd ? pwd : nullptr;
}

int main(int argc, char *argv[])
{
	auto uid = getuid();
	if(uid != 0) {
		std::cout << "This program must be run as root" << std::endl;
		return 1;
	}

	unsigned short port = 6666;

	if(argc > 1) {
		try {
			port = std::stoi(argv[1]);
		} catch(...) {
			std::cerr << "Port argument must be a number 1-65535" << std::endl;
			return 2;
		}
	}

	boost::asio::io_service io;
	net::server server(io, port);

	auto t = std::thread([&server]() {
		uint8_t own_type = 0x00;
		uint8_t scan_type = 0x01;
		uint8_t filter_type = 0;
		uint8_t filter_policy = 0x00;
		uint16_t interval = htobs(0x0010);
		uint16_t window = htobs(0x0010);
		uint8_t filter_dup = 0;

		auto dev = ble_init();

		int err = hci_le_set_scan_parameters(dev.sock, scan_type, interval, window,
										own_type, filter_policy, 1000);

		err = hci_le_set_scan_enable(dev.sock, 0x01, filter_dup, 1000);

		// Filter type 0x0
		ble_device_scan_fn(dev, 0, [&](bdaddr_t bdaddr) -> void {
			// on arrival
			for(auto& ep : server.endpoints()) {
				ep->send("unlock");
			}
		}, [&](bdaddr_t bdaddr) -> void {
			// on leave
			for(auto& ep : server.endpoints()) {
				ep->send("lock");
			}
		});

		err = hci_le_set_scan_enable(dev.sock, 0x00, filter_dup, 1000);
		close(dev.sock);
		hci_close_dev(dev.dev_id);
	});
	t.detach();

	server.run();
	io.run();

	return 0;
}

