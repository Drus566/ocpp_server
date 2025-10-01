######################################################
#              OCPP RPC protocol library             #
######################################################


# Library target
add_library(ocpp_rpc OBJECT
    RpcBase.cpp
    RpcClient.cpp
    RpcServer.cpp
    RpcPool.cpp
)

# Exported includes
target_include_directories(ocpp_rpc PUBLIC .)

# Dependencies
target_link_libraries(ocpp_rpc PUBLIC
    ocpp_ws
    ocpp_json
    ocpp_helpers
)
