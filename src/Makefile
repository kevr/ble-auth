
.PHONY: client server install clean

all: client server

client:
	$(MAKE) -C endpoint all

server:
	$(MAKE) -C server all

install:
	$(MAKE) -C server install
	$(MAKE) -C endpoint install
	bash ./install.sh
	$(MAKE) -C services install
	@echo "==========================================================="
	@echo
	@echo "On server: See /etc/ble-auth/authorized to setup auth devices"
	@echo "On endpoint(s): Place /usr/share/applications/ble-auth.desktop into ~/.config/autostart to enable automatically starting the ble-auth client"
	@echo
	@echo "==========================================================="
clean:
	$(MAKE) -C endpoint clean
	$(MAKE) -C server clean
