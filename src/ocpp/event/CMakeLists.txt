add_library(os_ocpp_event OBJECT 
	DefaultCentralSystemEventsHandler.cpp
)
target_include_directories(os_ocpp_event INTERFACE .)
target_link_libraries(os_ocpp_event 
	open-ocpp-dynamic

	os_ocpp_config
	# open-ocpp-static
	# cs_ocpp_config
	
	# cs_ocpp_connector
	# cs_ocpp_manager
	# cs_charge_station_dynamic
)