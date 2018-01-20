//
/// Bluetooth Low-Energy functions
//
#ifndef BLE_H
#define BLE_H

#include "session.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <sys/socket.h>
#include <bluetooth/bluetooth.h>
#include <bluetooth/hci.h>
#include <bluetooth/hci_lib.h>
#include <errno.h>
#include <signal.h>
#include <sys/param.h>
#include <sys/ioctl.h>
#include <ctype.h>
#include <fcntl.h>
#include <vector>
#include <memory>
#include <unordered_map>
#include <fstream>
#include <iostream>
#include <sys/types.h>
#include <pwd.h>

#define FLAGS_AD_TYPE 0x01
#define FLAGS_LIMITED_MODE_BIT 0x01
#define FLAGS_GENERAL_MODE_BIT 0x02

#define EIR_FLAGS                   0x01  /* flags */
#define EIR_UUID16_SOME             0x02  /* 16-bit UUID, more available */
#define EIR_UUID16_ALL              0x03  /* 16-bit UUID, all listed */
#define EIR_UUID32_SOME             0x04  /* 32-bit UUID, more available */
#define EIR_UUID32_ALL              0x05  /* 32-bit UUID, all listed */
#define EIR_UUID128_SOME            0x06  /* 128-bit UUID, more available */
#define EIR_UUID128_ALL             0x07  /* 128-bit UUID, all listed */
#define EIR_NAME_SHORT              0x08  /* shortened local name */
#define EIR_NAME_COMPLETE           0x09  /* complete local name */
#define EIR_TX_POWER                0x0A  /* transmit power level */
#define EIR_DEVICE_ID               0x10  /* device ID */

namespace ble
{
	struct device
	{
		int dev_id = -1;
		int sock = -1;
		bdaddr_t bdaddr;
	};
};


ble::device ble_init();
int ble_read_flags(uint8_t *flags, const uint8_t *data, size_t size);
int ble_report_filter(uint8_t procedure, le_advertising_info *info);

std::vector<bdaddr_t> get_whitelist();

// Function: void on_arrive(bdaddr_t)
template<typename F1, typename F2>
int ble_device_scan_fn(ble::device& bt, uint8_t filter_type, F1 on_arrive, F2 on_leave)
{
	int& sock = bt.sock;
	int dev_id = bt.dev_id;

	if(sock < 0)
		return sock;

	unsigned char buf[HCI_MAX_EVENT_SIZE], *ptr;
	struct hci_filter nf, of;
	struct sigaction sa;
	socklen_t olen;
	int len;

	olen = sizeof(of);
	if(getsockopt(sock, SOL_HCI, HCI_FILTER, &of, &olen) < 0) {
		printf("getsockopt failed\n");
		// fail
	}

	hci_filter_clear(&nf);
	hci_filter_set_ptype(HCI_EVENT_PKT, &nf);
	hci_filter_set_event(EVT_LE_META_EVENT, &nf);

	if(setsockopt(sock, SOL_HCI, HCI_FILTER, &nf, sizeof(nf)) < 0) {
		printf("setsockopt failed\n");
	}

	/*
	memset(&sa, 0, sizeof(sa));
	sa.sa_flags = SA_NOCLDSTOP;
	sa.sa_handler = sigint_handler;
	sigaction(SIGINT, &sa, NULL);

	auto env_vars = get_env_vars(bt.user);
	for(auto& var : env_vars) {
		setenv(var.first.c_str(), var.second.c_str(), 1);
	}
	*/

	auto authorized_devices = get_whitelist();
	if(authorized_devices.size() == 0)
		return -1;

	std::cout << "Authorized Devices:" << std::endl;
	for(auto& authed : authorized_devices) {
		char buf[32];
		ba2str(&authed, buf);
		std::cout << "    " << buf << std::endl;
	}

	std::unique_ptr<bluetooth::session> session {nullptr};
	bool found = false;
	printf("starting read loop\n");
	std::chrono::time_point<std::chrono::system_clock> tp;
	while(1) {
		evt_le_meta_event *meta;
		le_advertising_info *info;
		char addr[18];

		while((len = read(sock, buf, sizeof(buf))) < 0) {
			if(errno == EINTR /* && sig_received == SIGINT */) {
				len = 0;
				goto done;
			}

			if(errno == EAGAIN || errno == EINTR) {
				continue;
			}
			goto done;
		}

		ptr = buf + (1 + HCI_EVENT_HDR_SIZE);
		len -= (1 + HCI_EVENT_HDR_SIZE);
		meta = (evt_le_meta_event *)ptr;
		if(meta->subevent != 0x02)
			goto done;

		info = (le_advertising_info *)(meta->data + 1);
		if(ble_report_filter(filter_type, info)) {
			ba2str(&info->bdaddr, addr);
			for(auto& authorized : authorized_devices) {
				if(bacmp(&info->bdaddr, &authorized) == 0) {
					// Match!
					if(!found) {
						found = true;
						on_arrive(info->bdaddr);
						printf("added device to session: %s\n", addr);
					}
					tp = std::chrono::system_clock::now();
				}
			}
		}

		auto now = std::chrono::system_clock::now();
		std::chrono::duration<double> delta = (now - tp);
		if(found && delta.count() >= 3.0) {
			found = false;
			on_leave(info->bdaddr);
			printf("session closed\n");
		}
	}
	printf("finished loop\n");
done:
	setsockopt(sock, SOL_HCI, HCI_FILTER, &of, sizeof(of));

	if(len < 0)
		return -1;
	return 0;
}


#endif
