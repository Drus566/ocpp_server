
# JSON tools library is an interface wrapper for the rapidjson
# library which disable the warnings coming from the rapidjson's headers
# and provides some helper classes
add_library(ocpp_json OBJECT JsonValidator.cpp)
target_include_directories(ocpp_json PUBLIC .)
target_link_libraries(ocpp_json PUBLIC rapidjson)
