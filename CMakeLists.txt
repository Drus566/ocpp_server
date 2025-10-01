cmake_minimum_required(VERSION 3.10)

project(OcppServer DESCRIPTION "OcppServer Source C++ is ocpp server entity")

set(CMAKE_C_STANDARD 11)               
set(CMAKE_POSITION_INDEPENDENT_CODE ON) 

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/lib)

# Enable position independant code for dynamic library generation
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

add_subdirectory(3rdparty)

add_subdirectory(src)

add_library(ocpp_server_dynamic SHARED
   src/Version.cpp
)

add_executable(ocpp_server main.cpp)

target_link_libraries(ocpp_server PRIVATE
   stdc++fs
)

# include(CMakeLists_app.txt)
