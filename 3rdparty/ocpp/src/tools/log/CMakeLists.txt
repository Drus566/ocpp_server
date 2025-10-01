
# Log library
add_library(ocpp_log OBJECT
    LogDatabase.cpp
    Logger.cpp
)
target_include_directories(ocpp_log PUBLIC .)
target_link_libraries(ocpp_log PUBLIC ocpp_database)
