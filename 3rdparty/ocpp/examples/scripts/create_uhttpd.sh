#!/bin/sh
cp /etc/config/uhttpd /etc/config/uhttpd.bak
echo "config uhttpd 'custom'
	option redirect_https '1'
	option home '/ocppclient/www'
	option max_requests '3'
	option max_connections '100'
	list lua_prefix '/cgi-bin/luci=/usr/lib/lua/luci/sgi/uhttpd.lua'
	option http_keepalive '1'
	list listen_http '0.0.0.0:86'
	option rfc1918_filter '0'
	option cgi_prefix '/api'
	option script_timeout '120'
	option network_timeout '120'
	list interpreter '.info=/bin/sh'
" >> /etc/config/uhttpd.bak
mv /etc/config/uhttpd.bak /etc/config/uhttpd
/etc/init.d/uhttpd restart

