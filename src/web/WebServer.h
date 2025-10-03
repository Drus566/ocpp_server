#ifndef OS_WEB_WEBSERVER_H
#define OS_WEB_WEBSERVER_H

#include "OcppManager.h"

#include <libwebsockets.h>
#include <string>
#include <functional>
#include <unordered_map>
#include <memory>
#include <vector>
#include <fstream>
#include <sstream>
#include <atomic>
#include <chrono>

// RapidJSON
#include "json.h"

namespace os {
namespace web {

// Предварительное объявление
class WebServer;

// Типы колбэков
using HttpHandler = std::function<int(const std::string& uri, const std::string& method, 
                                     const std::unordered_map<std::string, std::string>& headers,
                                     const std::string& body, std::string& response)>;

using WebSocketConnectHandler = std::function<void(int connection_id)>;
using WebSocketMessageHandler = std::function<void(int connection_id, const std::string& message)>;
using WebSocketCloseHandler = std::function<void(int connection_id)>;

// RPC структуры
struct RpcRequest {
    std::string jsonrpc;
    std::string method;
    rapidjson::Document params;
    int id;
    bool has_params;
};

struct RpcResponse {
    std::string jsonrpc;
    std::string result;
    std::string error;
    int id;
};

class WebSocketConnection {
private:
    struct lws* wsi;
    int connection_id;
    std::vector<uint8_t> write_buffer;

public:
    WebSocketConnection(struct lws* wsi, int conn_id);
    
    void send(const std::string& message);
    void sendBinary(const std::vector<uint8_t>& data);
    int getId() const { return connection_id; }
    struct lws* getWsi() const { return wsi; }
    
    // Для внутреннего использования
    void appendToWriteBuffer(const std::string& message);
    std::vector<uint8_t> getWriteBuffer() const;
    void clearWriteBuffer();
};

class WebServer {
private:
    struct lws_context* context;
    int port;
    bool running;
    
    // Колбэки
    HttpHandler httpHandler;
    WebSocketConnectHandler wsConnectHandler;
    WebSocketMessageHandler wsMessageHandler;
    WebSocketCloseHandler wsCloseHandler;
    
    // Подключения WebSocket
    std::unordered_map<int, std::shared_ptr<WebSocketConnection>> connections;
    int next_connection_id;
    
    // RPC данные
    std::unordered_map<std::string, std::string> values_;
    std::atomic<int> request_counter_;
    std::chrono::steady_clock::time_point start_time_;
    
    // Системные метрики
    struct SystemMetrics {
        double currentPower;
        double voltage;
        double temperature;
        int connectedClients;
    } metrics_;
    
    // Протоколы (должны сохраняться в течение жизни контекста)
    static struct lws_protocols protocols_[3];
    
    // Статические методы для колбэков libwebsockets
    static int callback_http(struct lws* wsi, enum lws_callback_reasons reason, 
                            void* user, void* in, size_t len);

    static int callback_websocket(struct lws* wsi, enum lws_callback_reasons reason, 
                                 void* user, void* in, size_t len);
    
    // Внутренние методы
    void handleWebSocketConnect(struct lws* wsi);
    void handleWebSocketMessage(struct lws* wsi, void* in, size_t len);
    void handleWebSocketClose(struct lws* wsi);
    int generateConnectionId();
    
    // Инициализация протоколов
    void initializeProtocols();
    
    // Новые методы для HTTP обработки
    void sendHttpResponse(struct lws* wsi, const std::string& content, int status_code);
    // Обновленный метод отправки HTTP ответа
    void sendHttpResponse(struct lws* wsi, const std::string& content, int status_code, const std::string& mimeType);

    std::string generateDefaultResponse(const std::string& uri);

    // Метод для чтения файлов из папки html
    std::string readFile(const std::string &filePath);

    // Метод для определения MIME типа
    std::string getMimeType(const std::string &filePath);

    // Метод для обслуживания статических файлов
    bool serveStaticFile(struct lws *wsi, const std::string &uri);

    // RPC методы
    void handleRpcMessage(int connection_id, const std::string &message);

    RpcRequest parseRpcRequest(const std::string &json);
    std::string serializeRpcResponse(const RpcResponse &response);
    void sendRpcResponse(int connection_id, int request_id, const std::string &result);
    void sendRpcResponse(int connection_id, int request_id, const rapidjson::Value &result);
    void sendRpcError(int connection_id, int request_id, const std::string &error);
    void sendRpcNotification(const std::string &method, const rapidjson::Value &params);
    void broadcastValueChange(const std::string &key, const std::string &value);

    // RPC команды
    std::string handleGetValue(const rapidjson::Value &params);
    std::string handleGetStations();
    std::string handleGetStationStatus(const rapidjson::Value &params);
    std::string handleSetValue(const rapidjson::Value &params);
    std::string handleGetValues(const rapidjson::Value &params);
    std::string handleGetMetrics(const rapidjson::Value &params);
    std::string handleStartCharging(const rapidjson::Value &params);
    std::string handleStopCharging(const rapidjson::Value &params);
    std::string handleResetSystem(const rapidjson::Value &params);
    std::string handleSetMaxPower(const rapidjson::Value &params);

    // Вспомогательные методы
    void initializeValues();
    void updateMetrics();
    long getUptime() const;

    // RapidJSON утилиты
    rapidjson::Document createJsonDocument();
    std::string jsonToString(const rapidjson::Document &doc);
    std::string jsonValueToString(const rapidjson::Value &value);

    // const char* WebServer::get_callback_reason_name(enum lws_callback_reasons reason);

public:
    WebServer(os::ocpp::OcppManager& ocpp_manager, int port = 8080);
    ~WebServer();
    
    // Конфигурация
    void setHttpHandler(HttpHandler handler);
    void setWebSocketConnectHandler(WebSocketConnectHandler handler);
    void setWebSocketMessageHandler(WebSocketMessageHandler handler);
    void setWebSocketCloseHandler(WebSocketCloseHandler handler);
    
    // Управление сервером
    bool start();
    void stop();
    void run();
    
    // Управление подключениями
    void broadcast(const std::string& message);
    void sendToConnection(int connection_id, const std::string& message);
    void closeConnection(int connection_id);
    
    // Геттеры
    bool isRunning() const { return running; }
    int getPort() const { return port; }
    size_t getConnectionCount() const { return connections.size(); }

    os::ocpp::OcppManager& m_ocpp_manager;
    
    // Статические утилиты
    static std::unordered_map<std::string, std::string> parseHeaders(const std::string& headers);
    static std::string urlDecode(const std::string& encoded);
};

} // namespace web
} // namespace os

#endif // OS_WEB_WEBSERVER_H