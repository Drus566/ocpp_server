#ifndef CS_WEBSOCKET_SESSION_H
#define CS_WEBSOCKET_SESSION_H

#include <libwebsockets.h>
#include <string>
#include <vector>

namespace cs {
namespace web {

class WebSocketServer;

class WebSocketSession {
public:
	WebSocketSession(struct lws *wsi, WebSocketServer *server);
	~WebSocketSession();

	void onConnect();
	void onDisconnect();
	void onMessage(const std::string &message);
	bool sendMessage(const std::string &message);

	bool isConnected() const { return m_connected; }
	struct lws *getWsi() const { return m_wsi; }
	WebSocketServer *getServer() const { return m_server; }

private:
	struct lws *m_wsi;
	WebSocketServer *m_server;
	bool m_connected;
};

} // web
} // cs

#endif // CS_WEBSOCKET_SESSION_H