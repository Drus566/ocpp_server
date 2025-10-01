#include "HttpRequestHandler.h"
#include <fstream>
#include <iostream>
#include <sstream>

namespace cs {
namespace web {

HttpRequestHandler::HttpRequestHandler(const std::string &web_root_path) : web_root(web_root_path) {
	initializeMimeTypes();
}

void HttpRequestHandler::initializeMimeTypes() {
	mim_types[".html"] = "text/html";
	mim_types[".htm"] = "text/html";
	mim_types[".css"] = "text/css";
	mim_types[".js"] = "application/javascript";
	mim_types[".json"] = "application/json";
	mim_types[".png"] = "image/png";
	mim_types[".jpg"] = "image/jpeg";
	mim_types[".jpeg"] = "image/jpeg";
	mim_types[".gif"] = "image/gif";
	mim_types[".ico"] = "image/x-icon";
	mim_types[".txt"] = "text/plain";
}

std::string HttpRequestHandler::getMimeType(const std::string &filename) const {
	size_t dot_pos = filename.find_last_of('.');
	if (dot_pos != std::string::npos) {
		std::string extension = filename.substr(dot_pos);
		auto it = mim_types.find(extension);
		if (it != mim_types.end()) {
			return it->second;
		}
	}
	return "application/octet-stream";
}

bool HttpRequestHandler::fileExists(const std::string &path) const {
	std::ifstream file(path);
	return file.good();
}

int HttpRequestHandler::handleRequest(struct lws *wsi, const std::string &uri) {
	std::string filePath = web_root + uri;

	// Если запрос корневой, обслуживаем index.html
	if (uri == "/") {
		filePath = web_root + "/index.html";
	}

	if (!fileExists(filePath)) {
		std::cout << "File not found: " << filePath << std::endl;
		return 404;
	}

	std::string mimeType = getMimeType(filePath);

	if (lws_serve_http_file(wsi, filePath.c_str(), mimeType.c_str(), nullptr, 0) < 0) {
		return 500;
	}

	return 200;
}

std::string HttpRequestHandler::readFileContent(const std::string &filename) const {
	std::ifstream file(filename, std::ios::binary);
	if (!file) {
		return "";
	}

	std::stringstream buffer;
	buffer << file.rdbuf();
	return buffer.str();
}

} // web
} // cs