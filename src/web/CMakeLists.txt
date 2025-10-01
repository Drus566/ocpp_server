add_library(cs_web_server OBJECT 
	server/HttpRequestHandler.cpp
	server/WebSocketServer.cpp
	server/WebSocketSession.cpp
	CSWebServer.cpp
)

target_include_directories(cs_web_server PUBLIC server)

target_link_libraries(cs_web_server PUBLIC
   websockets
)

