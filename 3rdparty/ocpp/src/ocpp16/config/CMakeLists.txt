######################################################
#           OCPP configuration library               #
######################################################


# Library target
add_library(ocpp_config OBJECT
    InternalConfigManager.cpp)

# Exported includes
target_include_directories(ocpp_config INTERFACE .)

# Dependencies
target_link_libraries(ocpp_config PUBLIC
    ocpp_types
    ocpp_helpers
    ocpp_database
    ocpp_log
)
