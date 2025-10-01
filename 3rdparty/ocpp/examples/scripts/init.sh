#!/bin/sh
opkg update

mkdir -p /ocppclient/exchange
mkdir -p /ocppclient/www
mkdir -p /ocppclient/config
mkdir -p /ocppclient/certificates
mkdir -p /ocppclient/bin
mkdir -p /ocppclient/lib
mkdir -p /ocppclient/scripts

/ocppclient/scripts/update_certs.sh
/ocppclient/scripts/create_certs.sh
/ocppclient/scripts/create_initd.sh
/ocppclient/scripts/create_cron.sh
/ocppclient/scripts/create_uhttpd.sh


