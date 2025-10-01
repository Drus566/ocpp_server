######################################################
#            OCPP 2.0 centralsystem library          #
######################################################


# Library target
add_library(ocpp_centralsystem20 OBJECT
    CentralSystem20.cpp

    chargepoint/ChargePointHandler20.cpp
    chargepoint/ChargePointProxy20.cpp
)

# Exported includes
target_include_directories(ocpp_centralsystem20 PUBLIC interface)

# Private includes
target_include_directories(ocpp_centralsystem20 PRIVATE chargepoint
                                                   config)

# Dependencies
target_link_libraries(ocpp_centralsystem20
    ocpp_config
    ocpp_database
    ocpp_messages20
    ocpp_rpc
    ocpp_helpers
    ocpp_log
    ocpp_version
    ocpp_x509
    stdc++fs
)
