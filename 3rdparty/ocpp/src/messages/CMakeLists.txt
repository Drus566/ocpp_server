######################################################
#               OCPP messages library                #
######################################################


# Library target
add_library(ocpp_messages OBJECT
    MessageDispatcher.cpp
    GenericMessageSender.cpp
)

# Exported includes
target_include_directories(ocpp_messages PUBLIC . types)

# Dependencies
target_link_libraries(ocpp_messages PUBLIC
    ocpp_rpc
    ocpp_json
    ocpp_helpers
    ocpp_types
    ocpp_log
)
