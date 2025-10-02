# Создание OBJECT библиотеки
add_library(os_web_server OBJECT
    WebServer.cpp
)

target_include_directories(os_web_server PUBLIC .)

target_link_libraries(os_web_server PUBLIC
    websockets
    ocpp_json # библиотека из 3rdparty ocpp_json
    os_ocpp_manager
)
