add_library(os_ocpp_manager OBJECT OcppManager.cpp)
target_include_directories(os_ocpp_manager INTERFACE .)
target_link_libraries(os_ocpp_manager PUBLIC
	open-ocpp-dynamic

	# cs_ocpp_connector
	# open-ocpp-dynamic
	# cs_charge_station_data
	# cs_modbus
	# cs_types
	# cs_connectors
	os_ocpp_event
	
   #  examples_common20
   ocpp_centralsystem20

	# open-ocpp-static
	# cs_helpers
	os_ocpp_config
	# cs_helpers
)

# add_library(cs_ocpp_config INTERFACE)
# target_include_directories(cs_ocpp_config INTERFACE config)

# add_library(cs_ocpp OBJECT
#    # connector/OcppConnector.cpp
# 	# manager/OcppManager.cpp
# 	# event/DefaultChargePointEventsHandler.cpp
# )
# target_include_directories(cs_ocpp PUBLIC event)

# Dependencies
# target_link_libraries(cs_ocpp
# 	# open-ocpp-static
# 	cs_helpers
# 	open-ocpp-dynamic
# 	cs_ocpp_config
# 	cs_charge_station_dynamic

# )

# # Common utility class for examples library
# add_library(examples_common20 OBJECT
#     DefaultChargePointEventsHandler.cpp
#     DefaultBasicChargePointEventsHandler.cpp
#     DefaultCentralSystemEventsHandler.cpp
#     DefaultLocalControllerEventsHandler.cpp
# )
# target_include_directories(examples_common20 PUBLIC . config simulators)

# # Dependencies
# target_link_libraries(examples_common20 examples_common)

