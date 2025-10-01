######################################################
#            OCPP chargepoint library                #
######################################################


# Library target
add_library(ocpp_chargepoint OBJECT
    ChargePoint.cpp
    authent/AuthentCache.cpp
    authent/AuthentLocalList.cpp
    authent/AuthentManager.cpp
    config/ConfigManager.cpp
    connector/Connectors.cpp
    datatransfer/DataTransferManager.cpp
    iso15118/Iso15118Manager.cpp
    maintenance/MaintenanceManager.cpp
    metervalues/MeterValuesManager.cpp
    requestfifo/RequestFifo.cpp
    requestfifo/RequestFifoManager.cpp
    reservation/ReservationManager.cpp
    security/CaCertificatesDatabase.cpp
    security/CpCertificatesDatabase.cpp
    security/SecurityLogsDatabase.cpp
    security/SecurityManager.cpp
    smartcharging/ProfileDatabase.cpp
    smartcharging/SmartChargingManager.cpp
    status/StatusManager.cpp
    transaction/TransactionManager.cpp
    trigger/TriggerMessageManager.cpp
)

# Exported includes
target_include_directories(ocpp_chargepoint PUBLIC interface)

# Private includes
target_include_directories(ocpp_chargepoint PRIVATE authent
                                               config
                                               connector
                                               datatransfer
                                               interface
                                               iso15118
                                               maintenance
                                               metervalues
                                               requestfifo
                                               reservation
                                               security
                                               smartcharging
                                               status
                                               transaction
                                               trigger)

# Dependencies
target_link_libraries(ocpp_chargepoint PUBLIC
    ocpp_config
    ocpp_database
    ocpp_messages16
    ocpp_rpc
    ocpp_helpers
    ocpp_log
    ocpp_version
    ocpp_x509
)
