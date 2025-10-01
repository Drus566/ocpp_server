#include "CSWebServer.h"

#include <iostream>
#include <sstream>
#include <iomanip>

namespace cs {
namespace web {

CSWebServer::CSWebServer(int port) : WebSocketServer(port)
{
	// Устанавливаем callback-функции через базовый класс
	setMessageCallback([this](WebSocketSession *session, const std::string &message)
							 { this->handleMessage(session, message); });

	setConnectCallback([this](WebSocketSession *session)
							 { this->handleConnect(session); });

	setDisconnectCallback([this](WebSocketSession *session)
								 { this->handleDisconnect(session); });

	std::cout << "CSWebServer created on port " << port << std::endl;
}

void CSWebServer::handleConnect(WebSocketSession *session)
{
	std::cout << "New client connected. Total clients: " << getConnectedClients() << std::endl;

	// Отправляем приветственное сообщение
	std::string welcomeMsg = createWelcomeMessage(getConnectedClients());
	session->sendMessage(welcomeMsg);

	// Уведомляем всех клиентов о новом подключении
	std::string broadcastMsg = createBroadcastMessage();
	broadcastMessage(broadcastMsg);
}

void CSWebServer::handleDisconnect(WebSocketSession *session)
{
	std::cout << "Client disconnected. Total clients: " << getConnectedClients() << std::endl;

	// Уведомляем оставшихся клиентов об отключении
	if (getConnectedClients() > 0)
	{
		std::string notification = R"({
            "type": "client_disconnected",
            "message": "Клиент отключился",
            "remaining_clients": )" +
											std::to_string(getConnectedClients()) + "}";

		broadcastMessage(notification);
	}
}

void CSWebServer::handleMessage(WebSocketSession *session, const std::string &message)
{
	std::cout << "Processing message from client: " << message << std::endl;

	try
	{
		// Эхо-ответ с временной меткой
		std::string echoResponse = createEchoResponse(message);
		session->sendMessage(echoResponse);

		// Широковещательное сообщение для всех клиентов
		std::string broadcastMsg = R"({
            "type": "broadcast",
            "message": "Новое сообщение от клиента",
            "original_length": )" +
											std::to_string(message.length()) + R"(,
            "timestamp": )" + std::to_string(std::time(nullptr)) +
											"}";

		broadcastMessage(broadcastMsg);
	}
	catch (const std::exception &e)
	{
		std::cerr << "Error processing message: " << e.what() << std::endl;

		// Отправляем сообщение об ошибке
		std::string errorMsg = R"({
            "type": "error",
            "message": "Ошибка обработки сообщения"
        })";

		session->sendMessage(errorMsg);
	}
}

std::string CSWebServer::createWelcomeMessage(size_t clientCount) const
{
	std::time_t now = std::time(nullptr);
	std::stringstream ss;

	ss << R"({
        "type": "welcome",
        "message": "Добро пожаловать на CSWebServer!",
        "timestamp": )"
		<< now << R"(,
        "clients": )"
		<< clientCount << R"(,
        "server": "CSWebServer",
        "version": "1.0"
    })";

	return ss.str();
}

std::string CSWebServer::createEchoResponse(const std::string &originalMessage) const
{
	std::time_t now = std::time(nullptr);
	std::string escapedMessage;

	// Экранируем специальные символы JSON
	for (char c : originalMessage)
	{
		if (c == '"' || c == '\\')
		{
			escapedMessage += '\\';
		}
		escapedMessage += c;
	}

	std::stringstream ss;
	ss << R"({
        "type": "echo",
        "original": ")"
		<< escapedMessage << R"(",
        "timestamp": )"
		<< now << R"(,
        "processed": true
    })";

	return ss.str();
}

std::string CSWebServer::createBroadcastMessage() const
{
	std::time_t now = std::time(nullptr);

	std::stringstream ss;
	ss << R"({
        "type": "system_notification",
        "message": "Обновление статуса подключений",
        "timestamp": )"
		<< now << R"(,
        "active_clients": )"
		<< getConnectedClients() << R"(,
        "event": "client_connected"
    })";

	return ss.str();
}

} // web
} // cs