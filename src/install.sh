#!/bin/bash

CURDIR=$(dirname $(realpath $0))
echo $CURDIR

if [ ! -f /etc/ble-auth/authorized ]; then
	mkdir -p /etc/ble-auth
	install -m0755 $CURDIR/skel/authorized /etc/ble-auth
fi
