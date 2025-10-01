#################################################################################
#                               OCPP library                                    #
#################################################################################

cmake_minimum_required(VERSION 3.13)

project(OpenOCPP DESCRIPTION "Open Source C++ implementation of the OCPP 1.6 and 2.0.1 protocols"
                 VERSION 2.0.0
)

# Definitions for Version.h file
add_definitions(-DLIBOPENOCPP_MAJOR="${PROJECT_VERSION_MAJOR}")
add_definitions(-DLIBOPENOCPP_MINOR="${PROJECT_VERSION_MINOR}")
add_definitions(-DLIBOPENOCPP_FIX="${PROJECT_VERSION_PATCH}")

# Build options
include(CMakeLists_Options.txt)

# Warnings
set(WARNING_FLAGS "-Wall -Wextra -Werror")

# C/C++ flags
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

# Set compiler flags
set(CMAKE_C_FLAGS_DEBUG     "${CMAKE_C_FLAGS_DEBUG_INIT} ${WARNING_FLAGS} -O0 -g3 -ggdb3")
set(CMAKE_CXX_FLAGS_DEBUG   "${CMAKE_CXX_FLAGS_DEBUG_INIT} ${WARNING_FLAGS} -O0 -g3 -ggdb3")
set(CMAKE_C_FLAGS_RELEASE   "${CMAKE_C_FLAGS_RELEASE_INIT} ${WARNING_FLAGS} -O2 -DNDEBUG")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE_INIT} ${WARNING_FLAGS} -O2 -DNDEBUG")

# Enable position independant code for dynamic library generation
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# 3rd party
add_subdirectory(3rdparty)

# OpenSSL is mandatory
find_package(OpenSSL REQUIRED COMPONENTS SSL Crypto)

# libwesockets is mandatory if not built along the Open OCPP library
if(NOT ${BUILD_LWS_LIBRARY})
    find_package(PkgConfig)
    pkg_check_modules(LIB_WEBSOCKETS REQUIRED libwebsockets)
endif()

# Default output directories
# if(NOT DEFINED APP_DIR)
#     set(APP_DIR ${CMAKE_SOURCE_DIR}/app)
# endif()

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin/ocpp)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/lib/ocpp)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/lib/ocpp)

# if(NOT DEFINED BIN_DIR)
#     set(BIN_DIR ${CMAKE_SOURCE_DIR}/bin)
# endif()
# set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${BIN_DIR})
# set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${BIN_DIR})
# set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${BIN_DIR})

# Examples
# add_subdirectory(examples)

# Open OCPP library sources
add_subdirectory(src)

# Open OCPP static library
# if(${BUILD_STATIC_LIBRARY})
#     add_library(open-ocpp-static STATIC
#         src/version.cpp)
#     target_link_libraries(open-ocpp-static
#         centralsystem
#         chargepoint
#         localcontroller
#         centralsystem20
#         chargepoint20
#         localcontroller20
#         config
#         database
#         messages
#         messages16
#         messages20
#         types20
#         rpc
#         helpers
#         log
#         version
#         x509
#         json
#         ws
#         websockets
#     )
#     set_target_properties(open-ocpp-static PROPERTIES OUTPUT_NAME "open-ocpp_static")
#     set(OPEN_OCPP_STATIC_TARGET open-ocpp-static)
# endif()

# Open OCPP dynamic library
if (${BUILD_SHARED_LIBRARY})
    add_library(open-ocpp-dynamic SHARED
        src/version.cpp)
    target_link_libraries(open-ocpp-dynamic
        ocpp_centralsystem
        ocpp_chargepoint
        # ocpp_localcontroller
        ocpp_centralsystem20
        ocpp_chargepoint20
        # ocpp_localcontroller20
        ocpp_config
        ocpp_database
        ocpp_messages
        ocpp_messages16
        ocpp_messages20
        ocpp_types20
        ocpp_rpc
        ocpp_helpers
        ocpp_log
        ocpp_version
        ocpp_x509
        ocpp_json
        ocpp_ws
        websockets
    )
    set_target_properties(open-ocpp-dynamic PROPERTIES
        OUTPUT_NAME "open-ocpp"
        VERSION ${PROJECT_VERSION}
        SOVERSION ${PROJECT_VERSION_MAJOR}
    )
    set(OPEN_OCPP_SHARED_TARGET open-ocpp-dynamic)
endif()

# Install commands
include(GNUInstallDirs)

# file(GLOB_RECURSE PUBLIC_HEADERS
#      LIST_DIRECTORIES false RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} "${CMAKE_CURRENT_SOURCE_DIR}/src/*.h")
# file(GLOB OCPP16_SCHEMAS
#      LIST_DIRECTORIES false RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} "${CMAKE_CURRENT_SOURCE_DIR}/schemas/ocpp16/*.json")
# file(GLOB OCPP20_SCHEMAS
#      LIST_DIRECTORIES false RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} "${CMAKE_CURRENT_SOURCE_DIR}/schemas/ocpp20/*.json")

# install(TARGETS ${OPEN_OCPP_SHARED_TARGET} ${OPEN_OCPP_STATIC_TARGET}
#         RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}/ocpp"
#         LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}/ocpp"
#         ARCHIVE DESTINATION "${CMAKE_INSTALL_LIBDIR}/ocpp"
# )
# install(FILES ${PUBLIC_HEADERS} DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/openocpp)
# install(FILES ${OCPP16_SCHEMAS} DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/openocpp/schemas/ocpp16)
# install(FILES ${OCPP20_SCHEMAS} DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/openocpp/schemas/ocpp20)

# if(${INSTALL_RAPID_JSON})
#     file(GLOB RAPIDJSON_HEADERS
#          LIST_DIRECTORIES false RELATIVE ${CMAKE_SOURCE_DIR} "${CMAKE_SOURCE_DIR}/3rdparty/rapidjson/include/rapidjson/*.h")
#     file(GLOB RAPIDJSON_INTERNAL_HEADERS
#          LIST_DIRECTORIES false RELATIVE ${CMAKE_SOURCE_DIR} "${CMAKE_SOURCE_DIR}/3rdparty/rapidjson/include/rapidjson/internal/*.h")
#     file(GLOB RAPIDJSON_ERROR_HEADERS
#          LIST_DIRECTORIES false RELATIVE ${CMAKE_SOURCE_DIR} "${CMAKE_SOURCE_DIR}/3rdparty/rapidjson/include/rapidjson/error/*.h")
#     install(FILES ${RAPIDJSON_HEADERS} DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/openocpp/rapidjson)
#     install(FILES ${RAPIDJSON_INTERNAL_HEADERS} DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/openocpp/rapidjson/internal)
#     install(FILES ${RAPIDJSON_ERROR_HEADERS} DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/openocpp/rapidjson/error)
# endif()

# Generate pkgconfig files
# set(PKG_CONFIG_LIBDIR       "\${prefix}/lib")
# set(PKG_CONFIG_INCLUDEDIR   "\${prefix}/include/openocpp")
# set(PKG_CONFIG_LIBS         "-L\${libdir}")
# set(PKG_CONFIG_CFLAGS       "-I\${includedir}")

if(${BUILD_SHARED_LIBRARY})
    set(LIB_NAME                "open-ocpp")
    # configure_file(
    #   "${CMAKE_CURRENT_SOURCE_DIR}/deploy/libopen-ocpp.pc.in"
    #   "${CMAKE_CURRENT_BINARY_DIR}/libopen-ocpp.pc"
    # )
    # install(FILES "${CMAKE_BINARY_DIR}/libopen-ocpp.pc" DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig)
endif()

# if(${BUILD_STATIC_LIBRARY})
#     set(LIB_NAME                "open-ocpp_static")
#     configure_file(
#     "${CMAKE_CURRENT_SOURCE_DIR}/deploy/libopen-ocpp.pc.in"
#     "${CMAKE_CURRENT_BINARY_DIR}/libopen-ocpp_static.pc"
#     )
#     install(FILES "${CMAKE_BINARY_DIR}/libopen-ocpp_static.pc" DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig)
# endif()

# # Copy to app directory
# ADD_CUSTOM_COMMAND(TARGET open-ocpp-dynamic
#     POST_BUILD
#     COMMAND ${CMAKE_COMMAND} -E make_directory ${APP_DIR}/ocppclient/certificates
#     COMMAND ${CMAKE_COMMAND} -E make_directory ${APP_DIR}/ocppclient/scripts
#     COMMAND ${CMAKE_COMMAND} -E make_directory ${APP_DIR}/ocppclient/www
#     COMMAND ${CMAKE_COMMAND} -E make_directory ${APP_DIR}/ocppclient/www/api
#     COMMAND ${CMAKE_COMMAND} -E make_directory ${APP_DIR}/ocppclient/exchange
#     COMMAND ${CMAKE_COMMAND} -E make_directory ${APP_DIR}/ocppclient/schemas
#     COMMAND ${CMAKE_COMMAND} -E make_directory ${APP_DIR}/ocppclient/config
#     COMMAND ${CMAKE_COMMAND} -E make_directory ${APP_DIR}/ocppclient/database
#     COMMAND ${CMAKE_COMMAND} -E make_directory ${APP_DIR}/ocppclient/log
#     COMMAND ${CMAKE_COMMAND} -E make_directory ${APP_DIR}/ocppclient/temp

#     COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_LIST_DIR}/examples/certificates/* ${APP_DIR}/ocppclient/certificates
#     COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_LIST_DIR}/examples/scripts/* ${APP_DIR}/ocppclient/scripts
#     COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_LIST_DIR}/schemas/ocpp20/* ${APP_DIR}/ocppclient/schemas
#     # COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_LIST_DIR}/examples/chargepoint_client/config/universal_cp.ini ${APP_DIR}/ocppclient/config
#     # COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_LIST_DIR}/examples/chargepoint_client/config/universal_cp.ini ${APP_DIR}/ocppclient/config/universal_cp.ini.bak
#     # COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_LIST_DIR}/examples/chargepoint_client/config/modbus.json ${APP_DIR}/ocppclient/config
# )

# add_compile_options(-Wno-error=unused-function)
# add_compile_options(-Wno-error=unused-variable)

