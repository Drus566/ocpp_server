
# Database library
add_library(ocpp_database OBJECT 
    Database.cpp    
)
target_include_directories(ocpp_database PUBLIC .)
target_link_libraries(ocpp_database PUBLIC sqlite3)
