######################################################
#            OCPP chargepoint library                #
######################################################


# Library target
add_library(ocpp_chargepoint20 OBJECT
    ChargePoint20.cpp
    BasicChargePoint20.cpp

    connector/Connectors20.cpp
    devicemodel/DeviceModelManager20.cpp
    status/StatusManager20.cpp
    trigger/TriggerMessageManager20.cpp
)

# Exported includes
target_include_directories(ocpp_chargepoint20 PUBLIC interface devicemodel connector)

# Private includes
target_include_directories(ocpp_chargepoint20 PRIVATE config status trigger)

# Dependencies
target_link_libraries(ocpp_chargepoint20 PUBLIC
    ocpp_config
    ocpp_database
    ocpp_messages20
    ocpp_rpc
    ocpp_helpers
    ocpp_log
    ocpp_version
    ocpp_x509
)
