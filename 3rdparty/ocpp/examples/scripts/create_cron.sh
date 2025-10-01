#!/bin/sh
opkg install micrond

echo "50 23 * * * /ocppclient/scripts/ocpp_logs_clear.sh 60
*/5 * * * * /ocppclient/scripts/ocpp-watchdog
" > /usr/lib/micron.d/mycron

/etc/init.d/micrond restart