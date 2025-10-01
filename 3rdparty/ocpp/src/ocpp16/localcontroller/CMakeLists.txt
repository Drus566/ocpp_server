######################################################
#          OCPP localcontroller library              #
######################################################


# Library target
add_library(ocpp_localcontroller OBJECT
    LocalController.cpp

    centralsystem/CentralSystemHandler.cpp
    centralsystem/CentralSystemProxy.cpp

    chargepoint/ChargePointHandler.cpp
    chargepoint/ChargePointProxy.cpp
)

# Exported includes
target_include_directories(ocpp_localcontroller PUBLIC interface)

# Private includes
target_include_directories(ocpp_localcontroller PRIVATE .
                                                   centralsystem
                                                   chargepoint
                                                   config)

# Dependencies
target_link_libraries(ocpp_localcontroller
    ocpp_centralsystem
)
