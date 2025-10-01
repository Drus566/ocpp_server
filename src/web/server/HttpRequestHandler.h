#ifndef CS_HTTP_REQUEST_HANDLER_H
#define CS_HTTP_REQUEST_HANDLER_H

#include <libwebsockets.h>
#include <string>
#include <unordered_map>

namespace cs {
namespace web {

class HttpRequestHandler {
public:
	HttpRequestHandler(const std::string &webRootPath = "./html");

	int handleRequest(struct lws *wsi, const std::string &uri);
	std::string readFileContent(const std::string &filename) const;

private:
	std::string web_root;
	std::unordered_map<std::string, std::string> mim_types;

	void initializeMimeTypes();
	std::string getMimeType(const std::string &filename) const;
	bool fileExists(const std::string &path) const;
};

} // web
} // cs

#endif // CS_HTTP_REQUEST_HANDLER_H