#ifndef CS_WEB_SERVER_H
#define CS_WEB_SERVER_H

#include "WebSocketServer.h"

#include <string>
#include <ctime>

namespace cs {
namespace web {

class CSWebServer : public WebSocketServer {
public:
	CSWebServer(int port = 7681);
	virtual ~CSWebServer() = default;

private:
	void handleConnect(WebSocketSession *session) override;
	void handleDisconnect(WebSocketSession *session) override;
	void handleMessage(WebSocketSession *session, const std::string &message) override;

	std::string createWelcomeMessage(size_t clientCount) const;
	std::string createEchoResponse(const std::string &originalMessage) const;
	std::string createBroadcastMessage() const;
};

} // web
} // cs

#endif