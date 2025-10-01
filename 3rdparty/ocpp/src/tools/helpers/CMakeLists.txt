
# Helper library
add_library(ocpp_helpers OBJECT 
    IniFile.cpp
    StringHelpers.cpp
    Timer.cpp
    TimerPool.cpp
    WorkerThreadPool.cpp
)
target_include_directories(ocpp_helpers PUBLIC .)
