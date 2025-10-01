######################################################
#            OCPP 2.0 messages library               #
######################################################

# Library target
file (GLOB OCPP20_MESSAGES_SOURCE_FILES "*.cpp")
add_library(ocpp_messages20 OBJECT
    ${OCPP20_MESSAGES_SOURCE_FILES}
)

# Exported includes
target_include_directories(ocpp_messages20 PUBLIC .)

# Dependencies
target_link_libraries(ocpp_messages20 PUBLIC ocpp_types20 ocpp_messages)
