######################################################
#     Websockets library based on libwebsockets      #
#           (https://libwebsockets.org/)             #
######################################################


# Library target
add_library(ocpp_ws OBJECT
    Url.cpp
    WebsocketFactory.cpp
    libwebsockets/LibWebsocketClient.cpp
    libwebsockets/LibWebsocketClientPool.cpp
    libwebsockets/LibWebsocketServer.cpp
 )

# Private includes
target_include_directories(ocpp_ws PRIVATE libwebsockets)

# Exported includes
target_include_directories(ocpp_ws PUBLIC . stubs)

# Dependencies
target_link_libraries(ocpp_ws PUBLIC
    websockets
    ocpp_helpers
)
