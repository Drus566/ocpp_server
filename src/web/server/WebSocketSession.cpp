#include "WebSocketSession.h"
#include <iostream>
#include <cstring>

namespace cs {
namespace web {

WebSocketSession::WebSocketSession(struct lws* wsi, WebSocketServer* server) 
    : m_wsi(wsi), m_server(server), m_connected(true) {
    std::cout << "WebSocketSession created with server reference" << std::endl;
}

WebSocketSession::~WebSocketSession() {
	std::cout << "WebSocketSession destroyed" << std::endl;
}

void WebSocketSession::onConnect() {
	m_connected = true;
	std::cout << "Client connected" << std::endl;
}

void WebSocketSession::onDisconnect() {
	m_connected = false;
	std::cout << "Client disconnected" << std::endl;
}

void WebSocketSession::onMessage(const std::string& message) {
    std::cout << "Session processing message: " << message << std::endl;
    
    // Теперь у нас есть прямой доступ к серверу
    if (m_server) {
        // Вызываем метод сервера для обработки сообщения
        // Нужно добавить этот метод в WebSocketServer
        m_server->handleSessionMessage(this, message);
    }
}

bool WebSocketSession::sendMessage(const std::string& message) {
    if (!m_connected || !m_wsi) return false;
    
    unsigned char* buffer = new unsigned char[LWS_PRE + message.size()];
    memcpy(buffer + LWS_PRE, message.c_str(), message.size());
    
    int result = lws_write(m_wsi, buffer + LWS_PRE, message.size(), LWS_WRITE_TEXT);
    delete[] buffer;
    
    return result == static_cast<int>(message.size());
}

} // web
} // cs