
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
	@echo "Please see /etc/ble-auth/authorized to setup auth devices"

clean:
	$(MAKE) -C endpoint clean
	$(MAKE) -C server clean
