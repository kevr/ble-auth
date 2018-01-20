#include "ble.h"

ble::device ble_init()
{
	// Bluetooth specific code
	ble::device bt;

	int err, opt, dd;
	uint8_t own_type = 0x00;
	uint8_t scan_type = 0x01;
	uint8_t filter_type = 0;
	uint8_t filter_policy = 0x00;
	uint16_t interval = htobs(0x0010);
	uint16_t window = htobs(0x0010);
	uint8_t filter_dup = 0;

	bt.dev_id = hci_get_route(NULL);
	bt.sock = hci_open_dev(bt.dev_id);

	hci_devba(bt.dev_id, &bt.bdaddr);

	char buf[24];
	ba2str(&bt.bdaddr, buf);
	printf("device bdaddr: %s\n", buf);

	return bt;
}

int ble_read_flags(uint8_t *flags, const uint8_t *data, size_t size)
{
	size_t offset;

	if (!flags || !data)
		return -EINVAL;

	offset = 0;
	while (offset < size) {
		uint8_t len = data[offset];
		uint8_t type;

		/* Check if it is the end of the significant part */
		if (len == 0)
			break;

		if (len + offset > size)
			break;

		type = data[offset + 1];

		if (type == FLAGS_AD_TYPE) {
			*flags = data[offset + 2];
			return 0;
		}

		offset += 1 + len;
	}

	return -ENOENT;
}


int ble_report_filter(uint8_t procedure, le_advertising_info *info)
{
	uint8_t flags;

	/* If no discovery procedure is set, all reports are treat as valid */
	if (procedure == 0)
		return 1;

	/* Read flags AD type value from the advertising report if it exists */
	if (ble_read_flags(&flags, info->data, info->length))
		return 0;

	switch (procedure) {
	case 'l': /* Limited Discovery Procedure */
		if (flags & FLAGS_LIMITED_MODE_BIT)
			return 1;
		break;
	case 'g': /* General Discovery Procedure */
		if (flags & (FLAGS_LIMITED_MODE_BIT | FLAGS_GENERAL_MODE_BIT))
			return 1;
		break;
	default:
		fprintf(stderr, "Unknown discovery procedure\n");
	}

	return 0;
}

std::vector<bdaddr_t> get_whitelist()
{
	std::vector<bdaddr_t> wl;

	std::ifstream ifs("/etc/ble-auth/authorized", std::ios::in);
	std::string s;
	while(ifs.good()) {
		std::getline(ifs, s);
		if(s.size() > 4 && s[0] != '#') {
			bdaddr_t bdaddr;
			str2ba(s.c_str(), &bdaddr);
			wl.push_back(bdaddr);
		}
	}
	return wl;
}


