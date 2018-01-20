#!/bin/bash

(ps aux | grep -q 'ble-client') && (ble-client 2>&1 >/tmp/ble-client.log &)

