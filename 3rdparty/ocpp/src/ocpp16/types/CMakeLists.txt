######################################################
#             OCPP 1.6 types library                 #
######################################################


# This is an header only library
add_library(ocpp_types16 INTERFACE)
target_include_directories(ocpp_types16 INTERFACE .)
target_link_libraries(ocpp_types16 INTERFACE ocpp_types)