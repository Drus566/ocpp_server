# Создание OBJECT библиотеки
add_library(cs_web_server OBJECT
    WebServer.cpp
)

target_include_directories(cs_web_server PUBLIC .)

target_link_libraries(cs_web_server PUBLIC
    websockets
    rapidjson
)
