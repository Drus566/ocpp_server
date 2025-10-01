######################################################
#          OCPP 2.0 localcontroller library          #
######################################################


# Library target
add_library(ocpp_localcontroller20 OBJECT
    LocalController20.cpp

    centralsystem/CentralSystemHandler20.cpp
    centralsystem/CentralSystemProxy20.cpp

    chargepoint/ChargePointHandler20.cpp
    chargepoint/ChargePointProxy20.cpp
)

# Exported includes
target_include_directories(ocpp_localcontroller20 PUBLIC interface)

# Private includes
target_include_directories(ocpp_localcontroller20 PRIVATE .
                                                     centralsystem
                                                     chargepoint
                                                     config)

# Dependencies
target_link_libraries(ocpp_localcontroller20
    ocpp_centralsystem20
)
