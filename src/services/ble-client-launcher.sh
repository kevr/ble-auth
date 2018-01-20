#!/bin/bash

if ! ps aux | grep -q 'ble-client'; then
	ble-client &
fi

