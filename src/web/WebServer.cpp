#include "WebServer.h"

#include <iostream>
#include <sstream>
#include <cstring>
#include <algorithm>
#include <map>
#include <fstream>
#include <random>
#include <cmath>

namespace os {
namespace web {

const char* get_callback_reason_name(enum lws_callback_reasons reason) {
    switch (reason) {
        case LWS_CALLBACK_ESTABLISHED: return "ESTABLISHED";
        case LWS_CALLBACK_CLIENT_CONNECTION_ERROR: return "CLIENT_CONNECTION_ERROR";
        case LWS_CALLBACK_CLIENT_FILTER_PRE_ESTABLISH: return "CLIENT_FILTER_PRE_ESTABLISH";
        case LWS_CALLBACK_CLIENT_ESTABLISHED: return "CLIENT_ESTABLISHED";
        case LWS_CALLBACK_CLOSED: return "CLOSED";
        case LWS_CALLBACK_CLOSED_HTTP: return "CLOSED_HTTP";
        case LWS_CALLBACK_RECEIVE: return "RECEIVE";
        case LWS_CALLBACK_RECEIVE_PONG: return "RECEIVE_PONG";
        case LWS_CALLBACK_CLIENT_RECEIVE: return "CLIENT_RECEIVE";
        case LWS_CALLBACK_CLIENT_RECEIVE_PONG: return "CLIENT_RECEIVE_PONG";
        case LWS_CALLBACK_CLIENT_WRITEABLE: return "CLIENT_WRITEABLE";
        case LWS_CALLBACK_SERVER_WRITEABLE: return "SERVER_WRITEABLE";
        case LWS_CALLBACK_HTTP: return "HTTP";
        case LWS_CALLBACK_HTTP_BODY: return "HTTP_BODY";
        case LWS_CALLBACK_HTTP_BODY_COMPLETION: return "HTTP_BODY_COMPLETION";
        case LWS_CALLBACK_HTTP_FILE_COMPLETION: return "HTTP_FILE_COMPLETION";
        case LWS_CALLBACK_HTTP_WRITEABLE: return "HTTP_WRITEABLE";
        case LWS_CALLBACK_FILTER_NETWORK_CONNECTION: return "FILTER_NETWORK_CONNECTION";
        case LWS_CALLBACK_FILTER_HTTP_CONNECTION: return "FILTER_HTTP_CONNECTION";
        case LWS_CALLBACK_SERVER_NEW_CLIENT_INSTANTIATED: return "SERVER_NEW_CLIENT_INSTANTIATED";
        case LWS_CALLBACK_FILTER_PROTOCOL_CONNECTION: return "FILTER_PROTOCOL_CONNECTION";
        case LWS_CALLBACK_OPENSSL_LOAD_EXTRA_CLIENT_VERIFY_CERTS: return "OPENSSL_LOAD_EXTRA_CLIENT_VERIFY_CERTS";
        case LWS_CALLBACK_OPENSSL_LOAD_EXTRA_SERVER_VERIFY_CERTS: return "OPENSSL_LOAD_EXTRA_SERVER_VERIFY_CERTS";
        case LWS_CALLBACK_OPENSSL_PERFORM_CLIENT_CERT_VERIFICATION: return "OPENSSL_PERFORM_CLIENT_CERT_VERIFICATION";
        case LWS_CALLBACK_CLIENT_APPEND_HANDSHAKE_HEADER: return "CLIENT_APPEND_HANDSHAKE_HEADER";
        case LWS_CALLBACK_CONFIRM_EXTENSION_OKAY: return "CONFIRM_EXTENSION_OKAY";
        case LWS_CALLBACK_CLIENT_CONFIRM_EXTENSION_SUPPORTED: return "CLIENT_CONFIRM_EXTENSION_SUPPORTED";
        case LWS_CALLBACK_PROTOCOL_INIT: return "PROTOCOL_INIT";
        case LWS_CALLBACK_PROTOCOL_DESTROY: return "PROTOCOL_DESTROY";
        case LWS_CALLBACK_WSI_CREATE: return "WSI_CREATE";
        case LWS_CALLBACK_WSI_DESTROY: return "WSI_DESTROY";
        case LWS_CALLBACK_GET_THREAD_ID: return "GET_THREAD_ID";
        case LWS_CALLBACK_ADD_POLL_FD: return "ADD_POLL_FD";
        case LWS_CALLBACK_DEL_POLL_FD: return "DEL_POLL_FD";
        case LWS_CALLBACK_CHANGE_MODE_POLL_FD: return "CHANGE_MODE_POLL_FD";
        case LWS_CALLBACK_LOCK_POLL: return "LOCK_POLL";
        case LWS_CALLBACK_UNLOCK_POLL: return "UNLOCK_POLL";
        // case LWS_CALLBACK_OPENSSL_CONTEXT_REQUIRES_PRIVATE_KEY: return "OPENSSL_CONTEXT_REQUIRES_PRIVATE_KEY";
        case LWS_CALLBACK_USER: return "USER";
        default: return "UNKNOWN";
    }
}

// Статическая инициализация протоколов
struct lws_protocols WebServer::protocols_[3];

// WebSocketConnection implementation
WebSocketConnection::WebSocketConnection(lws* wsi, int conn_id) 
    : wsi(wsi), connection_id(conn_id) {}

void WebSocketConnection::send(const std::string& message) {
    appendToWriteBuffer(message);
    lws_callback_on_writable(wsi);
}

void WebSocketConnection::sendBinary(const std::vector<uint8_t>& data) {
    write_buffer.insert(write_buffer.end(), data.begin(), data.end());
    lws_callback_on_writable(wsi);
}

void WebSocketConnection::appendToWriteBuffer(const std::string& message) {
    std::vector<uint8_t> data(message.begin(), message.end());
    write_buffer.insert(write_buffer.end(), data.begin(), data.end());
}

std::vector<uint8_t> WebSocketConnection::getWriteBuffer() const {
    return write_buffer;
}

void WebSocketConnection::clearWriteBuffer() {
    write_buffer.clear();
}

// WebServer implementation

WebServer::WebServer(os::ocpp::OcppManager& manager, int port) 
    : m_ocpp_manager(manager), context(nullptr), port(port), running(false), next_connection_id(1) {
    initializeProtocols();
    initializeValues();
}


WebServer::~WebServer() {
    stop();
}

void WebServer::initializeProtocols() {
    std::cout << "=== Initializing Protocols ===" << std::endl;
    
    // HTTP протокол
    memset(&protocols_[0], 0, sizeof(lws_protocols));
    protocols_[0].name = "http-only";
    protocols_[0].callback = callback_http;
    protocols_[0].per_session_data_size = 0;
    
    // WebSocket протокол - КРИТИЧЕСКИ ВАЖНО!
    memset(&protocols_[1], 0, sizeof(lws_protocols));
    protocols_[1].name = "websocket";  // Должно быть "websocket"
    protocols_[1].callback = callback_websocket;
    protocols_[1].per_session_data_size = sizeof(void*);
    protocols_[1].rx_buffer_size = 4096;
    protocols_[1].tx_packet_size = 4096;
    
    // Завершающий нулевой элемент
    memset(&protocols_[2], 0, sizeof(lws_protocols));
    protocols_[2].name = nullptr;
    protocols_[2].callback = nullptr;
    protocols_[2].per_session_data_size = 0;
    
    std::cout << "WebSocket protocol configured:" << std::endl;
    std::cout << "  Name: " << protocols_[1].name << std::endl;
    std::cout << "  per_session_data_size: " << protocols_[1].per_session_data_size << std::endl;
    std::cout << "  rx_buffer_size: " << protocols_[1].rx_buffer_size << std::endl;
    std::cout << "=== Protocols Initialized ===" << std::endl;
}

int WebServer::generateConnectionId() {
    return next_connection_id++;
}

void WebServer::setHttpHandler(HttpHandler handler) {
    httpHandler = handler;
}

void WebServer::setWebSocketConnectHandler(WebSocketConnectHandler handler) {
    wsConnectHandler = handler;
}

void WebServer::setWebSocketMessageHandler(WebSocketMessageHandler handler) {
    wsMessageHandler = handler;
}

void WebServer::setWebSocketCloseHandler(WebSocketCloseHandler handler) {
    wsCloseHandler = handler;
}

bool WebServer::start() {
    if (running) {
        std::cout << "Server is already running" << std::endl;
        return true;
    }
    
    char cwd[1024];
    if (getcwd(cwd, sizeof(cwd)) != NULL) {
        std::cout << "Current working directory: " << cwd << std::endl;
    }
    
    struct lws_context_creation_info info;
    memset(&info, 0, sizeof(info));
    
    info.port = port;
    info.protocols = protocols_;
    info.gid = -1;
    info.uid = -1;
    info.user = this;
    
    // ВАЖНО: Явно указываем слушать на всех интерфейсах
    info.options = LWS_SERVER_OPTION_HTTP_HEADERS_SECURITY_BEST_PRACTICES_ENFORCE;
    info.iface = NULL; // Слушаем на 0.0.0.0 (все интерфейсы)
    
    std::cout << "🚀 Starting server on port " << port << " (all interfaces: 0.0.0.0)" << std::endl;
    
    context = lws_create_context(&info);
    if (!context) {
        std::cerr << "❌ Failed to create libwebsockets context" << std::endl;
        return false;
    }
    
    running = true;
    
    std::cout << "✅ Server started successfully on all interfaces!" << std::endl;
    std::cout << "📍 Test URLs:" << std::endl;
    std::cout << "   WSL: http://localhost:" << port << "/" << std::endl;
    std::cout << "   Windows: http://<wsl-ip>:" << port << "/" << std::endl;
    
    return true;
}

void WebServer::stop() {
    if (!running) return;
    
    running = false;
    
    if (context) {
        lws_context_destroy(context);
        context = nullptr;
    }
    
    connections.clear();
    std::cout << "WebServer stopped" << std::endl;
}

void WebServer::run() {
    if (!running) {
        std::cerr << "Server is not running. Call start() first." << std::endl;
        return;
    }
    
    std::cout << "WebServer event loop started" << std::endl;
    
    int count = 0;
    while (running) {
        lws_service(context, 50);
        count++;
        
        if (count % 200 == 0) { // Реже выводим сообщения
            std::cout << "WebServer running... Connections: " << connections.size() << std::endl;
        }
    }
    
    std::cout << "WebServer event loop stopped" << std::endl;
}

void WebServer::broadcast(const std::string& message) {
    for (auto& conn : connections) {
        conn.second->send(message);
    }
}

void WebServer::sendToConnection(int connection_id, const std::string& message) {
    auto it = connections.find(connection_id);
    if (it != connections.end()) {
        it->second->send(message);
    }
}

void WebServer::closeConnection(int connection_id) {
    auto it = connections.find(connection_id);
    if (it != connections.end()) {
        lws_close_reason(it->second->getWsi(), LWS_CLOSE_STATUS_NORMAL, 
                        (unsigned char*)"Closed by server", 16);
        connections.erase(it);
    }
}

int WebServer::callback_http(struct lws* wsi, enum lws_callback_reasons reason, 
                            void* user, void* in, size_t len) {
    WebServer* server = static_cast<WebServer*>(lws_context_user(lws_get_context(wsi)));
    
    if (!server) {
        return -1;
    }
    
    switch (reason) {
        case LWS_CALLBACK_HTTP: {
            const char* uri = static_cast<const char*>(in);
            std::cout << "HTTP request: " << uri << std::endl;
            
            std::string requestUri(uri);
            if (requestUri == "/") {
                requestUri = "/index.html";
            }
            
            if (server->serveStaticFile(wsi, requestUri)) {
                return 0;
            }
            
            std::string response = server->generateDefaultResponse(requestUri);
            server->sendHttpResponse(wsi, response, 200);
            return 0;
        }
            
        default:
            break;
    }
    
    return 0;
}

int WebServer::callback_websocket(struct lws* wsi, enum lws_callback_reasons reason, 
                                 void* user, void* in, size_t len) {
    WebServer* server = static_cast<WebServer*>(lws_context_user(lws_get_context(wsi)));
    
    if (!server) {
        std::cerr << "ERROR: Server instance not found!" << std::endl;
        return -1;
    }
    
    switch (reason) {
        case LWS_CALLBACK_ESTABLISHED:
            std::cout << "🎉 WebSocket connection ESTABLISHED" << std::endl;
            server->handleWebSocketConnect(wsi);
            break;
            
        case LWS_CALLBACK_RECEIVE:
            std::cout << "📨 WebSocket MESSAGE RECEIVED - Length: " << len << std::endl;
            if (in && len > 0) {
                std::string message(static_cast<const char*>(in), len);
                std::cout << "Message content: " << message << std::endl;
                server->handleWebSocketMessage(wsi, in, len);
            }
            break;
            
        case LWS_CALLBACK_SERVER_WRITEABLE: {
            std::cout << "WebSocket WRITABLE" << std::endl;
            auto* conn_ptr = static_cast<WebSocketConnection**>(user);
            if (conn_ptr && *conn_ptr) {
                auto buffer = (*conn_ptr)->getWriteBuffer();
                if (!buffer.empty()) {
                    unsigned char* buf = new unsigned char[LWS_PRE + buffer.size()];
                    memcpy(buf + LWS_PRE, buffer.data(), buffer.size());
                    lws_write(wsi, buf + LWS_PRE, buffer.size(), LWS_WRITE_TEXT);
                    delete[] buf;
                    (*conn_ptr)->clearWriteBuffer();
                    std::cout << "Data sent to WebSocket" << std::endl;
                }
            }
            break;
        }
            
        case LWS_CALLBACK_CLOSED:
            std::cout << "WebSocket CLOSED" << std::endl;
            server->handleWebSocketClose(wsi);
            break;
            
        default:
            // Пропускаем логи для неважных событий
            break;
    }
    
    return 0;
}

void WebServer::sendHttpResponse(struct lws* wsi, const std::string& content, int status_code) {
    sendHttpResponse(wsi, content, status_code, "text/html"); // Вызов перегруженного метода
}

// Обновленный метод отправки HTTP ответа
// Новый перегруженный метод с MIME type
void WebServer::sendHttpResponse(struct lws* wsi, const std::string& content, int status_code, const std::string& mimeType) {
    // Сначала формируем полный ответ
    const char* status_text = "OK";
    if (status_code == 404) status_text = "Not Found";
    else if (status_code == 400) status_text = "Bad Request";
    else if (status_code == 500) status_text = "Internal Server Error";

    // Формируем заголовки
    std::string headers = 
        "HTTP/1.1 " + std::to_string(status_code) + " " + status_text + "\r\n"
        "Content-Type: " + mimeType + "\r\n"
        "Content-Length: " + std::to_string(content.length()) + "\r\n"
        "Connection: close\r\n"
        "Server: cs-web-server\r\n"
        "\r\n";

    // Полный ответ = заголовки + контент
    std::string full_response = headers + content;
    
    // Выделяем буфер достаточного размера
    size_t total_size = LWS_PRE + full_response.length();
    unsigned char* buffer = new unsigned char[total_size];
    
    // Копируем данные в буфер (после LWS_PRE)
    memcpy(buffer + LWS_PRE, full_response.c_str(), full_response.length());
    
    // Отправляем
    lws_write(wsi, buffer + LWS_PRE, full_response.length(), LWS_WRITE_HTTP);
    
    std::cout << "HTTP response sent: " << status_code << " " << status_text 
              << ", Content-Type: " << mimeType 
              << ", Content-Length: " << content.length() 
              << ", Total sent: " << full_response.length() << std::endl;
    
    delete[] buffer;
}

// Обновите generateDefaultResponse метод
std::string WebServer::generateDefaultResponse(const std::string& uri) {
    if (uri == std::string("/") || uri == std::string("/index.html")) {
        // Пытаемся прочитать index.html из папки
        std::string content = readFile("web/index.html");
        if (!content.empty()) {
            return content;
        }
        // Fallback на встроенный HTML
        return R"(<!DOCTYPE html>
            <html>
            <head>
                <title>OS Web Server - Fallback</title>
                <style>
                    body { font-family: Arial, sans-serif; margin: 40px; }
                    .container { max-width: 800px; margin: 0 auto; }
                </style>
            </head>
            <body>
                <div class="container">
                    <h1>OS Web Server - Fallback Mode</h1>
                    <p>Static files not found. Running in fallback mode.</p>
                    <p>Please check that the 'web/' directory exists with static files.</p>
                </div>
            </body>
            </html>)";
                } else if (uri == std::string("/api/status")) {
                    return R"({
                "status": "running",
                "service": "cs-web-server",
                "port": 8080,
                "connections": )" + std::to_string(connections.size()) + R"(,
                "timestamp": )" + std::to_string(time(nullptr)) + R"(
            })";
    } 
    else {
        // Пытаемся прочитать 404.html из папки
        std::string content = readFile("web/404.html");
        if (!content.empty()) {
            return content;
        }
        // Fallback на встроенный HTML
        return R"(<!DOCTYPE html>
            <html>
            <head>
                <title>404 Not Found</title>
                <style>
                    body { font-family: Arial, sans-serif; margin: 40px; }
                    h1 { color: #d32f2f; }
                </style>
            </head>
            <body>
                <h1>404 Not Found</h1>
                <p>The requested URL was not found on this server.</p>
                <p><a href="/">Return to homepage</a></p>
            </body>
            </html>)";
    }
}

void WebServer::handleWebSocketConnect(struct lws* wsi) {
    std::cout << "🎯 Handling WebSocket connection..." << std::endl;
    
    // Получаем user data
    void** user_data = static_cast<void**>(lws_wsi_user(wsi));
    if (!user_data) {
        std::cerr << "❌ Cannot get user data from WSI" << std::endl;
        return;
    }
    
    int conn_id = generateConnectionId();
    auto connection = std::make_shared<WebSocketConnection>(wsi, conn_id);
    
    // Сохраняем указатель
    *user_data = connection.get();
    connections[conn_id] = connection;
    
    std::cout << "✅ New WebSocket connection ID: " << conn_id << std::endl;
    std::cout << "✅ Connection stored at: " << *user_data << std::endl;
    std::cout << "✅ Total connections: " << connections.size() << std::endl;
    
    // Отправляем приветственное сообщение
    std::string welcome = R"({
        "jsonrpc": "2.0",
        "method": "connected", 
        "params": {
            "connectionId": )" + std::to_string(conn_id) + R"(,
            "message": "WebSocket connection established"
        }
    })";
    
    connection->send(welcome);
    lws_callback_on_writable(wsi);
    
    std::cout << "✅ Welcome message queued" << std::endl;
}

void WebServer::handleWebSocketMessage(struct lws* wsi, void* in, size_t len) {
    auto** user_data = static_cast<WebSocketConnection**>(lws_wsi_user(wsi));
    
    std::cout << "=== WebSocket Message Received ===" << std::endl;
    std::cout << "WebSocket pointer: " << wsi << std::endl;
    std::cout << "User data pointer: " << user_data << std::endl;
    
    if (!user_data) {
        std::cerr << "ERROR: User data is null!" << std::endl;
        return;
    }
    
    if (!*user_data) {
        std::cerr << "ERROR: WebSocketConnection pointer is null!" << std::endl;
        return;
    }
    
    int conn_id = (*user_data)->getId();
    std::string message(static_cast<char*>(in), len);
    
    std::cout << "Connection ID: " << conn_id << std::endl;
    std::cout << "Message length: " << len << std::endl;
    std::cout << "Message content: " << message << std::endl;
    std::cout << "=================================" << std::endl;
    
    // Пытаемся обработать как JSON-RPC
    try {
        handleRpcMessage(conn_id, message);
    } catch (const std::exception& e) {
        std::cerr << "RPC processing error: " << e.what() << std::endl;
        std::cerr << "Original message: " << message << std::endl;
        
        // Если не JSON-RPC, используем старый обработчик
        if (wsMessageHandler) {
            wsMessageHandler(conn_id, message);
        } else {
            // Эхо-ответ по умолчанию
            std::string response = "[" + std::to_string(time(nullptr)) + "] Echo: " + message;
            sendToConnection(conn_id, response);
        }
    }
}

void WebServer::handleWebSocketClose(struct lws* wsi) {
    auto** user_data = static_cast<WebSocketConnection**>(lws_wsi_user(wsi));
    if (!user_data || !*user_data) return;
    
    int conn_id = (*user_data)->getId();
    
    if (wsCloseHandler) {
        wsCloseHandler(conn_id);
    }
    
    connections.erase(conn_id);
    std::cout << "WebSocket connection closed: " << conn_id << std::endl;
}

// Статические утилиты
std::unordered_map<std::string, std::string> WebServer::parseHeaders(const std::string& headers) {
    std::unordered_map<std::string, std::string> result;
    std::istringstream iss(headers);
    std::string line;
    
    while (std::getline(iss, line)) {
        size_t pos = line.find(':');
        if (pos != std::string::npos) {
            std::string key = line.substr(0, pos);
            std::string value = line.substr(pos + 1);
            key.erase(0, key.find_first_not_of(" \t"));
            key.erase(key.find_last_not_of(" \t") + 1);
            value.erase(0, value.find_first_not_of(" \t"));
            value.erase(value.find_last_not_of(" \t") + 1);
            result[key] = value;
        }
    }
    
    return result;
}

std::string WebServer::urlDecode(const std::string& encoded) {
    std::string result;
    result.reserve(encoded.size());
    
    for (size_t i = 0; i < encoded.size(); ++i) {
        if (encoded[i] == '%' && i + 2 < encoded.size()) {
            int value;
            std::istringstream iss(encoded.substr(i + 1, 2));
            if (iss >> std::hex >> value) {
                result += static_cast<char>(value);
                i += 2;
            } else {
                result += encoded[i];
            }
        } else if (encoded[i] == '+') {
            result += ' ';
        } else {
            result += encoded[i];
        }
    }
    
    return result;
}

std::string WebServer::readFile(const std::string& filePath) {
    std::cout << "Reading file: " << filePath << std::endl;
    
    std::ifstream file(filePath, std::ios::binary);
    if (!file) {
        std::cout << "✗ Cannot open file: " << filePath << std::endl;
        return "";
    }
    
    // Получаем размер файла
    file.seekg(0, std::ios::end);
    std::streamsize size = file.tellg();
    file.seekg(0, std::ios::beg);
    
    if (size <= 0) {
        std::cout << "✗ File is empty: " << filePath << std::endl;
        return "";
    }
    
    std::cout << "File size: " << size << " bytes" << std::endl;
    
    // Читаем содержимое
    std::vector<char> buffer(size);
    if (!file.read(buffer.data(), size)) {
        std::cout << "✗ Failed to read file: " << filePath << std::endl;
        return "";
    }
    
    std::cout << "✓ Successfully read file: " << filePath << std::endl;
    return std::string(buffer.data(), buffer.size());
}

// Метод для определения MIME типа
std::string WebServer::getMimeType(const std::string& filePath) {
    std::string extension = filePath.substr(filePath.find_last_of(".") + 1);
    
    static std::unordered_map<std::string, std::string> mimeTypes = {
        {"html", "text/html"},
        {"htm", "text/html"},
        {"css", "text/css"},
        {"js", "application/javascript"},
        {"json", "application/json"},
        {"png", "image/png"},
        {"jpg", "image/jpeg"},
        {"jpeg", "image/jpeg"},
        {"gif", "image/gif"},
        {"svg", "image/svg+xml"},
        {"ico", "image/x-icon"},
        {"txt", "text/plain"}
    };
    
    // Приводим расширение к нижнему регистру
    std::string extLower = extension;
    std::transform(extLower.begin(), extLower.end(), extLower.begin(), ::tolower);
    
    auto it = mimeTypes.find(extLower);
    return it != mimeTypes.end() ? it->second : "text/plain";
}

// Метод для обслуживания статических файлов
bool WebServer::serveStaticFile(struct lws* wsi, const std::string& uri) {
    std::string basePath = "web";
    std::string filePath = basePath + uri;

    std::cout << "Trying to serve static file: " << filePath << std::endl;

    // Если путь заканчивается на /, добавляем index.html
    if (filePath.back() == '/')
    {
        filePath += "index.html";
    }

    // Безопасность
    if (filePath.find("..") != std::string::npos)
    {
        std::cout << "Security violation: path contains '..'" << std::endl;
        return false;
    }

    // Пробуем разные пути
    std::vector<std::string> pathsToTry = {
        filePath,
        "../" + filePath,
        "../../" + filePath,
        "app/" + filePath,
        "../app/" + filePath};

    for (const auto &path : pathsToTry)
    {
        std::cout << "Trying path: " << path << std::endl;
        std::string content = readFile(path);
        if (!content.empty())
        {
            std::string mimeType = getMimeType(path);
            std::cout << "✓ Serving: " << path << " (" << content.length() << " bytes)" << std::endl;

            // Отправляем ответ
            sendHttpResponse(wsi, content, 200, mimeType);
            return true;
        }
    }

    std::cout << "✗ Static file not found: " << filePath << std::endl;
    return false;
}

// Инициализация значений по умолчанию
void WebServer::initializeValues() {
    values_["systemStatus"] = "running";
    values_["maxPower"] = "50";
    values_["chargingEnabled"] = "false";
    values_["currentSession"] = "0";
    values_["totalEnergy"] = "0";
    values_["firmwareVersion"] = "1.0.0";
    values_["operator"] = "OS System";
    
    // Инициализация метрик
    metrics_.currentPower = 0.0;
    metrics_.voltage = 230.0;
    metrics_.temperature = 25.0;
    metrics_.connectedClients = 0;
    
    start_time_ = std::chrono::steady_clock::now();
}

// Получение времени работы
long WebServer::getUptime() const {
    auto now = std::chrono::steady_clock::now();
    return std::chrono::duration_cast<std::chrono::seconds>(now - start_time_).count();
}

// Обновление метрик (симуляция реальных данных)
void WebServer::updateMetrics() {
    static std::random_device rd;
    static std::mt19937 gen(rd());
    static std::uniform_real_distribution<> power_dis(0.0, 45.0);
    static std::uniform_real_distribution<> voltage_dis(225.0, 235.0);
    static std::uniform_real_distribution<> temp_dis(20.0, 35.0);
    
    // Обновляем метрики только если зарядка активна
    if (values_["chargingEnabled"] == "true") {
        metrics_.currentPower = power_dis(gen);
        metrics_.voltage = voltage_dis(gen);
        metrics_.temperature = temp_dis(gen) + metrics_.currentPower * 0.1; // Температура зависит от мощности
    } else {
        metrics_.currentPower = 0.0;
        metrics_.voltage = 230.0;
        metrics_.temperature = 25.0;
    }
    
    metrics_.connectedClients = connections.size();
}

// RapidJSON утилиты
rapidjson::Document WebServer::createJsonDocument() {
    rapidjson::Document doc;
    doc.SetObject();
    return doc;
}

std::string WebServer::jsonToString(const rapidjson::Document& doc) {
    rapidjson::StringBuffer buffer;
    rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
    doc.Accept(writer);
    return buffer.GetString();
}

std::string WebServer::jsonValueToString(const rapidjson::Value& value) {
    if (value.IsString()) {
        return value.GetString();
    } else if (value.IsInt()) {
        return std::to_string(value.GetInt());
    } else if (value.IsDouble()) {
        return std::to_string(value.GetDouble());
    } else if (value.IsBool()) {
        return value.GetBool() ? "true" : "false";
    } else {
        rapidjson::StringBuffer buffer;
        rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
        value.Accept(writer);
        return buffer.GetString();
    }
}

// Парсинг JSON RPC запроса с использованием RapidJSON
RpcRequest WebServer::parseRpcRequest(const std::string& json) {
    RpcRequest request;
    rapidjson::Document doc;
    
    // Парсим JSON
    rapidjson::ParseResult result = doc.Parse(json.c_str());
    if (!result) {
        throw std::runtime_error("JSON parse error: " + 
            std::string(rapidjson::GetParseError_En(result.Code())) + 
            " at offset " + std::to_string(result.Offset()));
    }
    
    // Проверяем что это объект
    if (!doc.IsObject()) {
        throw std::runtime_error("Expected JSON object");
    }
    
    // Извлекаем jsonrpc версию
    if (doc.HasMember("jsonrpc") && doc["jsonrpc"].IsString()) {
        request.jsonrpc = doc["jsonrpc"].GetString();
    } else {
        throw std::runtime_error("Missing or invalid 'jsonrpc' field");
    }
    
    // Извлекаем метод
    if (doc.HasMember("method") && doc["method"].IsString()) {
        request.method = doc["method"].GetString();
    } else {
        throw std::runtime_error("Missing or invalid 'method' field");
    }
    
    // Извлекаем ID
    if (doc.HasMember("id") && doc["id"].IsInt()) {
        request.id = doc["id"].GetInt();
    } else {
        throw std::runtime_error("Missing or invalid 'id' field");
    }
    
    // Извлекаем параметры (опционально)
    if (doc.HasMember("params")) {
        request.has_params = true;
        request.params.CopyFrom(doc["params"], request.params.GetAllocator());
    } else {
        request.has_params = false;
        request.params.SetObject();
    }
    
    return request;
}

// Сериализация JSON RPC ответа
std::string WebServer::serializeRpcResponse(const RpcResponse& response) {
    rapidjson::Document doc = createJsonDocument();
    auto& allocator = doc.GetAllocator();
    
    doc.AddMember("jsonrpc", rapidjson::Value().SetString("2.0", allocator), allocator);
    doc.AddMember("id", response.id, allocator);
    
    if (!response.error.empty()) {
        doc.AddMember("error", rapidjson::Value().SetString(response.error.c_str(), allocator), allocator);
    } else {
        doc.AddMember("result", rapidjson::Value().SetString(response.result.c_str(), allocator), allocator);
    }
    
    return jsonToString(doc);
}

// Отправка RPC ответа
void WebServer::sendRpcResponse(int connection_id, int request_id, const std::string& result) {
    RpcResponse response;
    response.jsonrpc = "2.0";
    response.result = result;
    response.id = request_id;
    
    std::string response_json = serializeRpcResponse(response);
    sendToConnection(connection_id, response_json);
}

// В реализации:
void WebServer::sendRpcResponse(int connection_id, int request_id, const rapidjson::Value& result) {
    rapidjson::Document doc = createJsonDocument();
    auto& allocator = doc.GetAllocator();
    
    doc.AddMember("jsonrpc", rapidjson::Value().SetString("2.0", allocator), allocator);
    doc.AddMember("id", request_id, allocator);
    
    // Копируем результат как объект
    rapidjson::Value result_copy;
    result_copy.CopyFrom(result, allocator);
    doc.AddMember("result", result_copy, allocator);
    
    std::string response_json = jsonToString(doc);
    sendToConnection(connection_id, response_json);
}

// Отправка RPC ошибки
void WebServer::sendRpcError(int connection_id, int request_id, const std::string& error) {
    RpcResponse response;
    response.jsonrpc = "2.0";
    response.error = error;
    response.id = request_id;
    
    std::string response_json = serializeRpcResponse(response);
    sendToConnection(connection_id, response_json);
}

// Отправка RPC уведомления
void WebServer::sendRpcNotification(const std::string& method, const rapidjson::Value& params) {
    rapidjson::Document doc = createJsonDocument();
    auto& allocator = doc.GetAllocator();
    
    doc.AddMember("jsonrpc", rapidjson::Value().SetString("2.0", allocator), allocator);
    doc.AddMember("method", rapidjson::Value().SetString(method.c_str(), allocator), allocator);
    
    // Копируем параметры
    rapidjson::Value params_copy;
    params_copy.CopyFrom(params, allocator);
    doc.AddMember("params", params_copy, allocator);
    
    std::string notification_json = jsonToString(doc);
    broadcast(notification_json);
}

// Широковещательное уведомление об изменении значения
void WebServer::broadcastValueChange(const std::string& key, const std::string& value) {
    rapidjson::Document doc = createJsonDocument();
    auto& allocator = doc.GetAllocator();
    
    doc.AddMember("key", rapidjson::Value().SetString(key.c_str(), allocator), allocator);
    doc.AddMember("value", rapidjson::Value().SetString(value.c_str(), allocator), allocator);
    
    sendRpcNotification("valueChanged", doc);
}

// Основной обработчик RPC сообщений
void WebServer::handleRpcMessage(int connection_id, const std::string& message) {
    std::cout << "=== Processing RPC Message ===" << std::endl;
    std::cout << "Connection: " << connection_id << std::endl;
    std::cout << "Raw message: " << message << std::endl;
    
    try {
        RpcRequest request = parseRpcRequest(message);
        
        std::cout << "Parsed RPC request:" << std::endl;
        std::cout << "  JSON-RPC: " << request.jsonrpc << std::endl;
        std::cout << "  Method: " << request.method << std::endl;
        std::cout << "  ID: " << request.id << std::endl;
        std::cout << "  Has params: " << request.has_params << std::endl;
        
        std::string result;
        
        // Обработка команд
        if (request.method == "getValue") {
            std::cout << "Handling getValue command" << std::endl;
            result = handleGetValue(request.params);
        }


        else if (request.method == "GetStations") {
            std::cout << "Handling GetStations command" << std::endl;
            result = handleGetStations();
        }
        else if (request.method == "GetStationStatus") {
            std::cout << "Handling GetStationStatus command" << std::endl;
            result = handleGetStationStatus(request.params);
        }
        else if (request.method == "GetConnectorStatus")
        {
            std::cout << "Handling GetConnectorStatus command" << std::endl;
            result = handleGetConnectorStatus(request.params);
        }

        else if (request.method == "setValue") {
            std::cout << "Handling setValue command" << std::endl;
            result = handleSetValue(request.params);
        }
        else if (request.method == "getValues") {
            std::cout << "Handling getValues command" << std::endl;
            result = handleGetValues(request.params);
        }
        else if (request.method == "getMetrics") {
            std::cout << "Handling getMetrics command" << std::endl;
            result = handleGetMetrics(request.params);
        }
        else if (request.method == "ping") {
            std::cout << "Handling ping command" << std::endl;
            result = "pong";
        }
        else if (request.method == "startCharging") {
            std::cout << "Handling startCharging command" << std::endl;
            result = handleStartCharging(request.params);
        }
        else if (request.method == "stopCharging") {
            std::cout << "Handling stopCharging command" << std::endl;
            result = handleStopCharging(request.params);
        }
        else if (request.method == "resetSystem") {
            std::cout << "Handling resetSystem command" << std::endl;
            result = handleResetSystem(request.params);
        }
        else if (request.method == "setMaxPower") {
            std::cout << "Handling setMaxPower command" << std::endl;
            result = handleSetMaxPower(request.params);
        }
        else {
            std::cout << "Unknown method: " << request.method << std::endl;
            throw std::runtime_error("Unknown method: " + request.method);
        }
        
        std::cout << "Command result: " << result << std::endl;
        sendRpcResponse(connection_id, request.id, result);
        
    } catch (const std::exception& e) {
        std::cerr << "RPC error: " << e.what() << std::endl;
        
        // Получаем ID запроса для отправки ошибки
        try {
            RpcRequest request = parseRpcRequest(message);
            sendRpcError(connection_id, request.id, e.what());
        } catch (...) {
            // Если не удалось распарсить ID, отправляем ошибку с ID = -1
            sendRpcError(connection_id, -1, e.what());
        }
    }
    
    std::cout << "=== RPC Processing Complete ===" << std::endl;
}

// Реализация RPC команд с RapidJSON
std::string WebServer::handleGetStations() {    
    std::vector<std::string> charge_points = m_ocpp_manager.getChargePointIds();

    if (charge_points.size() == 0) {
        for (int i = 0; i < 3; i++) {
            charge_points.push_back("ST000" + std::to_string(i));
        }
    }

    rapidjson::Document doc = createJsonDocument();
    auto &allocator = doc.GetAllocator();
    rapidjson::Value arr(rapidjson::kArrayType);

    for (const auto &s : charge_points) {
        rapidjson::Value str_val;
        str_val.SetString(s.c_str(), static_cast<rapidjson::SizeType>(s.length()), allocator);
        arr.PushBack(str_val, allocator);
    }

    doc.AddMember("stations", arr, allocator);
    return jsonToString(doc);
}

// Реализация RPC команд с RapidJSON
std::string WebServer::handleGetStationStatus(const rapidjson::Value& params) {
    if (!params.IsObject() || !params.HasMember("station_id") || !params["station_id"].IsString()) {
        throw std::runtime_error("Invalid parameters: expected object with 'station_id' string");
    }

    std::string station_id = params["station_id"].GetString();

    // Данные
    std::string station_status = "ok";
    int connector_count = 2;
    int max_power = 150;

    if(station_id == "ST0001") {
        station_status = "alarm";
        connector_count = 3;
        max_power = 120;
    }
    else if(station_id == "ST0002") {
        station_status = "off";
        connector_count = 4;
        max_power = 180;
    }

    // JSON
    rapidjson::Document doc = createJsonDocument();
    auto &allocator = doc.GetAllocator();

    doc.AddMember("status", rapidjson::Value(station_status.c_str(), static_cast<rapidjson::SizeType>(station_status.length()), allocator).Move(), allocator);
    doc.AddMember("connector_count", connector_count, allocator);
    doc.AddMember("max_power", max_power, allocator);

    rapidjson::Value connectors(rapidjson::kArrayType);

    for (int i = 0; i < connector_count; i++) {
        std::string type = "CHADEMO";
        std::string status = "charge";
        float meter = 33.1;
        float power = 22.1;
        if (i == 1) {
            type = "CCS2";
            status = "alarm";
            meter = 0;
            power = 0;
        }
        else if (i == 2) {
            type = "GBT";
            status = "ok";
            meter = 0;
            power = 0;
        }
        else if (i == 3) {
            type = "TYPE2";
            status = "off";
            meter = 0;
            power = 0;
        }
        rapidjson::Value connector(rapidjson::kObjectType);
        connector.AddMember("id", i + 1, allocator);
        connector.AddMember("type", rapidjson::Value(type.c_str(), static_cast<rapidjson::SizeType>(type.length()), allocator).Move(), allocator);
        connector.AddMember("status", rapidjson::Value(status.c_str(), static_cast<rapidjson::SizeType>(status.length()), allocator).Move(), allocator);
        connector.AddMember("meter", meter, allocator);
        connector.AddMember("power", power, allocator);
        connectors.PushBack(connector, allocator);
    }

    doc.AddMember("connectors", connectors, allocator);
    return jsonToString(doc);
}

std::string WebServer::handleGetConnectorStatus(const rapidjson::Value& params) {
    if (!params.IsObject() || !params.HasMember("station_id") || !params["station_id"].IsString()) {
        throw std::runtime_error("Invalid parameters: expected object with 'station_id' string");
    }

    if (!params.IsObject() || !params.HasMember("connector_id") || !params["connector_id"].IsInt()) {
        throw std::runtime_error("Invalid parameters: expected object with 'connector_id' int");
    }

    std::string station_id = params["station_id"].GetString();
    int connector_id = params["connector_id"].GetInt();

    // Данные
    std::string type = "CHADEMO";
    std::string status = "charge";
    float meter = 33.1;
    float power = 22.1;
    if (connector_id == 2) {
        type = "CCS2";
        status = "alarm";
        meter = 0;
        power = 0;
    }
    else if (connector_id == 3) {
        type = "GBT";
        status = "ok";
        meter = 0;
        power = 0;
    }
    else if (connector_id == 4) {
        type = "TYPE2";
        status = "off";
        meter = 0;
        power = 0;
    }

    // JSON
    rapidjson::Document doc = createJsonDocument();
    auto &allocator = doc.GetAllocator();

    doc.AddMember("status", rapidjson::Value(status.c_str(), static_cast<rapidjson::SizeType>(status.length()), allocator).Move(), allocator);
    doc.AddMember("type", rapidjson::Value(type.c_str(), static_cast<rapidjson::SizeType>(type.length()), allocator).Move(), allocator);
    doc.AddMember("meter", meter, allocator);
    doc.AddMember("power", power, allocator);

    return jsonToString(doc);
}


// Реализация RPC команд с RapidJSON
std::string WebServer::handleGetValue(const rapidjson::Value& params) {
    if (!params.IsObject() || !params.HasMember("key") || !params["key"].IsString()) {
        throw std::runtime_error("Invalid parameters: expected object with 'key' string");
    }
    
    std::string key = params["key"].GetString();
    auto value_it = values_.find(key);
    if (value_it == values_.end()) {
        throw std::runtime_error("Key not found: " + key);
    }
    
    return value_it->second;
}

std::string WebServer::handleSetValue(const rapidjson::Value& params) {
    if (!params.IsObject() || !params.HasMember("key") || !params["key"].IsString() || 
        !params.HasMember("value")) {
        throw std::runtime_error("Invalid parameters: expected object with 'key' and 'value'");
    }
    
    std::string key = params["key"].GetString();
    std::string value;
    
    if (params["value"].IsString()) {
        value = params["value"].GetString();
    } else {
        value = jsonValueToString(params["value"]);
    }
    
    values_[key] = value;
    
    // Уведомляем всех клиентов об изменении
    broadcastValueChange(key, value);
    
    return "OK";
}

std::string WebServer::handleGetValues(const rapidjson::Value& params) {
    updateMetrics();
    
    rapidjson::Document doc = createJsonDocument();
    auto& allocator = doc.GetAllocator();
    
    doc.SetObject();
    for (const auto& pair : values_) {
        doc.AddMember(
            rapidjson::Value().SetString(pair.first.c_str(), allocator),
            rapidjson::Value().SetString(pair.second.c_str(), allocator),
            allocator
        );
    }
    
    // Добавляем текущие метрики как объекты
    doc.AddMember("currentPower", metrics_.currentPower, allocator);
    doc.AddMember("voltage", metrics_.voltage, allocator);
    doc.AddMember("temperature", metrics_.temperature, allocator);
    doc.AddMember("uptime", getUptime(), allocator);
    
    // ВАЖНО: возвращаем объект, а не строку
    return jsonToString(doc);
}


std::string WebServer::handleGetMetrics(const rapidjson::Value& params) {
    updateMetrics();
    
    rapidjson::Document doc = createJsonDocument();
    auto& allocator = doc.GetAllocator();
    
    doc.SetObject();
    doc.AddMember("currentPower", metrics_.currentPower, allocator);
    doc.AddMember("voltage", metrics_.voltage, allocator);
    doc.AddMember("temperature", metrics_.temperature, allocator);
    doc.AddMember("connectedClients", metrics_.connectedClients, allocator);
    doc.AddMember("uptime", getUptime(), allocator);
    
    // Используем перегруженный sendRpcResponse
    // Но так как мы должны вернуть string, делаем так:
    rapidjson::Document response_doc = createJsonDocument();
    response_doc.SetObject();
    response_doc.AddMember("jsonrpc", "2.0", allocator);
    response_doc.AddMember("id", 1, allocator); // Будет перезаписан
    
    rapidjson::Value result_value;
    result_value.CopyFrom(doc, allocator);
    response_doc.AddMember("result", result_value, allocator);
    
    return jsonToString(response_doc);
}

std::string WebServer::handleStartCharging(const rapidjson::Value& params) {
    values_["chargingEnabled"] = "true";
    values_["systemStatus"] = "charging";
    
    // Симулируем начало зарядки
    metrics_.currentPower = 10.0; // Начальная мощность
    
    broadcastValueChange("chargingEnabled", "true");
    broadcastValueChange("systemStatus", "charging");

    std::cout << "Start Charging" << std::endl;
    
    return "Charging started";
}

std::string WebServer::handleStopCharging(const rapidjson::Value& params) {
    values_["chargingEnabled"] = "false";
    values_["systemStatus"] = "ready";
    
    // Сбрасываем мощность
    metrics_.currentPower = 0.0;
    
    broadcastValueChange("chargingEnabled", "false");
    broadcastValueChange("systemStatus", "ready");
    
    return "Charging stopped";
}

std::string WebServer::handleResetSystem(const rapidjson::Value& params) {
    // Сбрасываем систему к начальным значениям
    initializeValues();
    
    broadcastValueChange("systemStatus", "reset");
    
    return "System reset completed";
}

std::string WebServer::handleSetMaxPower(const rapidjson::Value& params) {
    if (!params.IsObject() || !params.HasMember("value") || !params["value"].IsInt()) {
        throw std::runtime_error("Invalid parameters: expected object with 'value' as integer");
    }
    
    int maxPower = params["value"].GetInt();
    if (maxPower < 1 || maxPower > 100) {
        throw std::runtime_error("Max power must be between 1 and 100 kW");
    }
    
    values_["maxPower"] = std::to_string(maxPower);
    broadcastValueChange("maxPower", std::to_string(maxPower));
    
    return "Max power set to " + std::to_string(maxPower) + " kW";
}

} // namespace web
} // namespace os