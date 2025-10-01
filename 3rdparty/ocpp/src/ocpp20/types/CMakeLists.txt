######################################################
#              OCPP 2.0 types library                #
######################################################


# Library target
file (GLOB OCPP20_TYPES_SOURCE_FILES "*.cpp")
add_library(ocpp_types20 OBJECT
    ${OCPP20_TYPES_SOURCE_FILES}
)

# Exported includes
target_include_directories(ocpp_types20 PUBLIC .)

# Dependencies
target_link_libraries(ocpp_types20 PUBLIC ocpp_types ocpp_messages)
