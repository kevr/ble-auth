#!/bin/bash

if [ "$UID" -eq "0" ]; then
	echo "do not run this script as root; run this script as a gnome user"
	exit 1
fi

echo "Bootstrapping user autolaunch script"
mkdir -p $HOME/.config/autostart
cp /usr/share/applications/ble-auth.desktop $HOME/.config/autostart/
echo "Please relogin to enable the ble-auth client"

