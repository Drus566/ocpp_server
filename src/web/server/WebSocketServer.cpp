#include "WebSocketServer.h"

#include <iostream>
#include <csignal>
#include <cstring>
#include <algorithm>

namespace cs {
namespace web {

// Статические члены для обработки сигналов
static std::atomic<bool> g_interrupted{false};

static void signal_handler(int signum) {
	g_interrupted = true;
}

// Протоколы для libwebsockets
static struct lws_protocols protocols[] = {
	{
		"http-only",
		WebSocketServer::httpCallback,
		0,
	 	0
	},
	{
		"websocket",
		WebSocketServer::websocketCallback, 
		sizeof(WebSocketSession *), 
		WebSocketSession::BUFFER_SIZE
	},
	{ nullptr, nullptr, 0, 0 }
};

WebSocketServer::WebSocketServer(int port) : m_context(nullptr), 
															m_protocols(nullptr), 
															m_http_handler(nullptr), 
															m_port(port), 
															m_running(false) {

	m_http_handler = new HttpRequestHandler();

	// Установка обработчиков сигналов
	signal(SIGINT, signal_handler);
	signal(SIGTERM, signal_handler);
}

WebSocketServer::~WebSocketServer() {
	stop();

	if (m_http_handler) {
		delete m_http_handler;
	}

	if (m_protocols) {
		delete[] m_protocols;
	}
}

bool WebSocketServer::initialize() {
	struct lws_context_creation_info info;
	memset(&info, 0, sizeof(info));

	info.port = m_port;
	info.protocols = protocols;
	info.gid = -1;
	info.uid = -1;
	info.options = LWS_SERVER_OPTION_HTTP_HEADERS_SECURITY_BEST_PRACTICES_ENFORCE;

	m_context = lws_create_context(&info);
	if (!m_context) {
		std::cerr << "Failed to create libwebsockets context" << std::endl;
		return false;
	}

	std::cout << "WebSocket server initialized on port " << m_port << std::endl;
	return true;
}

void WebSocketServer::run() {
	if (!m_context) {
		std::cerr << "Server not initialized. Call initialize() first." << std::endl;
		return;
	}

	m_running = true;
	std::cout << "WebSocket server started. Press Ctrl+C to stop." << std::endl;

	while (m_running && !g_interrupted) {
		lws_service(m_context, 50);

		// Дополнительная обработка здесь при необходимости
	}

	stop();
}

void WebSocketServer::stop() {
	m_running = false;

	if (m_context) {
		lws_context_destroy(m_context);
		m_context = nullptr;
	}

	// Очистка сессий
	std::lock_guard<std::mutex> lock(m_session_mutex);
	for (auto session : m_sessions) {
		delete session;
	}
	m_sessions.clear();

	std::cout << "WebSocket server stopped" << std::endl;
}

// Статические callback-функции
int WebSocketServer::httpCallback(struct lws *wsi, lws_callback_reasons reason, void *user, void *in, size_t len) {
	if (reason == LWS_CALLBACK_HTTP) {
		const char *uri = static_cast<const char *>(in);
		WebSocketSession **sessionPtr = static_cast<WebSocketSession **>(user);

		// Получаем экземпляр сервера из контекста
		struct lws_context *context = lws_get_context(wsi);
		struct lws_vhost *vhost = lws_get_vhost(wsi);

		// Это упрощенный подход - в реальном приложении нужно хранить сервер в контексте
		if (uri)	{
			// Просто логируем запрос
			std::cout << "HTTP request: " << uri << std::endl;
		}

		// Всегда возвращаем 0, чтобы libwebsockets обработал запрос
		return 0;
	}
	return 0;
}

int WebSocketServer::websocketCallback(struct lws *wsi, lws_callback_reasons reason, void *user, void *in, size_t len) {
	WebSocketSession **session_ptr = static_cast<WebSocketSession **>(user);

	switch (reason) {
		case LWS_CALLBACK_ESTABLISHED: {
			*session_ptr = new WebSocketSession(wsi);
			// Вызов обработчика подключения будет в основном коде
			std::cout << "WebSocket connection established" << std::endl;
			break;
		}

		case LWS_CALLBACK_RECEIVE: {
         if (*session_ptr && in && len > 0) {
             std::string message(static_cast<const char*>(in), len);
				 (*session_ptr)->onMessage(message);
			}
         break;
		}

		case LWS_CALLBACK_CLOSED: {
         if (*session_ptr) {
				(*session_ptr)->onDisconnect();
				delete *session_ptr;
				*session_ptr = nullptr;
			}
         break;
		}

		default:
			break;
	}

	return 0;
}

void WebSocketServer::addSession(WebSocketSession *session) {
	std::lock_guard<std::mutex> lock(m_session_mutex);
	m_sessions.push_back(session);
	onClientConnected(session); // Вызываем виртуальный метод

	if (m_connect_callback) {
		m_connect_callback(session);
	}
}

void WebSocketServer::removeSession(WebSocketSession *session) {
	std::lock_guard<std::mutex> lock(m_session_mutex);
	auto it = std::find(m_sessions.begin(), m_sessions.end(), session);
	if (it != m_sessions.end()) {
		onClientDisconnected(session); // Вызываем виртуальный метод
		m_sessions.erase(it);

		if (m_disconnect_callback) {
			m_disconnect_callback(session);
		}
	}
}

void WebSocketServer::broadcastMessage(const std::string &message) {
	std::lock_guard<std::mutex> lock(m_session_mutex);
	for (auto session : m_sessions) {
		if (session->isConnected()) {
			session->sendMessage(message);
		}
	}
}

size_t WebSocketServer::getConnectedClients() {
	std::lock_guard<std::mutex> lock(m_session_mutex);
	return m_sessions.size();
}

} // web
} // cs