#!/bin/sh

echo "#!/bin/sh /etc/rc.common

START=99
STOP=10

BIN_PATH=/ocppclient/bin/chargepoint_client
CONFIG_PATH=/ocppclient/config/universal_cp.ini
DATABASE_PATH=/ocppclient/database/*

start() {
        stop
        \$BIN_PATH \"-f\" /ocppclient/log/app.log &
}

stop() {
        PID=\$(ps | awk '/[c]hargepoint_client/{print \$1}' | head -n 1) > /dev/null 2>&1
        kill -9 \$PID > /dev/null 2>&1
}

restart() {
        stop
        rm -rf \$DATABASE_PATH
        \$BIN_PATH \"-d\" \"-r\" \"-f\" /ocppclient/log/app.log &
}
" > /etc/init.d/ocppclientd

chmod +x /etc/init.d/ocppclientd
/etc/init.d/ocppclientd enable
/etc/init.d/ocppclientd start
