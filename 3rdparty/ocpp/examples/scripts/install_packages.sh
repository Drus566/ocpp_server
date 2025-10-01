#!/bin/sh
opkg install micrond
opkg install libopenssl
opkg install openssl-util 
opkg install libstdcpp
opkg install libstdcpp6
opkg install libsqlite3
opkg install libmodbus
opkg install --force-reinstall ca-certificates
opkg install --force-reinstall ca-bundle
