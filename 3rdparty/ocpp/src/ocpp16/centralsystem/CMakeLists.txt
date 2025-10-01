######################################################
#            OCPP centralsystem library              #
######################################################


# Library target
add_library(ocpp_centralsystem OBJECT
    CentralSystem.cpp

    chargepoint/ChargePointHandler.cpp
    chargepoint/ChargePointProxy.cpp
)

# Exported includes
target_include_directories(ocpp_centralsystem PUBLIC interface)

# Private includes
target_include_directories(ocpp_centralsystem PRIVATE chargepoint
                                                 config)

# Dependencies
target_link_libraries(ocpp_centralsystem
    ocpp_config
    ocpp_database
    ocpp_messages16
    ocpp_rpc
    ocpp_helpers
    ocpp_log
    ocpp_version
    ocpp_x509
)
