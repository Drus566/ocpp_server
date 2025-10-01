#ifndef CS_WEBSOCKET_SERVER_H
#define CS_WEBSOCKET_SERVER_H

#include "WebSocketSession.h"
#include "HttpRequestHandler.h"

#include <libwebsockets.h>
#include <vector>
#include <string>
#include <functional>
#include <mutex>
#include <atomic>

namespace cs {
namespace web {

// Предварительное объявление
// class WebSocketSession;
// class HttpRequestHandler;

class WebSocketServer {
public:
	using MessageCallback = std::function<void(WebSocketSession*, const std::string &)>;
	using ConnectionCallback = std::function<void(WebSocketSession *)>;

	WebSocketServer(int port = 7681);
	virtual ~WebSocketServer();

	bool initialize();
	void run();
	void stop();

	// Управление сессиями
	void addSession(WebSocketSession *session);
	void removeSession(WebSocketSession *session);
	void broadcastMessage(const std::string &message);

	// Callback setters
	void setMessageCallback(MessageCallback callback) { m_message_callback = callback; }
	void setConnectCallback(ConnectionCallback callback) { m_connect_callback = callback; }
	void setDisconnectCallback(ConnectionCallback callback) { m_disconnect_callback = callback; }

	int getPort() const { return m_port; }
	bool isRunning() const { return m_running; }
	size_t getConnectedClients();

	// Статические callback-функции для libwebsockets
	static int httpCallback(struct lws *wsi, lws_callback_reasons reason, void *user, void *in, size_t len);
	static int websocketCallback(struct lws *wsi, lws_callback_reasons reason, void *user, void *in, size_t len);

private:
	struct lws_context *m_context;
	struct lws_protocols *m_protocols;
	HttpRequestHandler *m_http_handler;

	std::vector<WebSocketSession *> m_sessions;
	std::mutex m_session_mutex;

	MessageCallback m_message_callback;
	ConnectionCallback m_connect_callback;
	ConnectionCallback m_disconnect_callback;

	// Виртуальные методы для переопределения в производных классах
	virtual void onClientConnected(WebSocketSession *session);
	virtual void onClientDisconnected(WebSocketSession *session);
	virtual void onMessageReceived(WebSocketSession *session, const std::string &message);

	std::atomic<bool> m_running;
	int m_port;


	// Нестатические обработчики
	int handleHttpRequest(struct lws *wsi, const std::string &uri);
	void handleWebSocketConnect(WebSocketSession *session);
	void handleWebSocketMessage(WebSocketSession *session, const std::string &message);
	void handleWebSocketDisconnect(WebSocketSession *session);
};

} // web
} // cs

#endif // CS_WEBSOCKET_SERVER_H