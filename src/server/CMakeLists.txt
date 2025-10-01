add_library(server OBJECT
   Server.cpp
)

target_include_directories(server PUBLIC .)

target_link_libraries(server
   stdc++fs
)