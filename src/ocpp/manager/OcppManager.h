#ifndef OS_OCPP_MANAGER_H
#define OS_OCPP_MANAGER_H

// #include "ChargeStation.h"

#include "CentralSystemDemoConfig.h"
#include "DefaultCentralSystemEventsHandler.h"
#include "ICentralSystem20.h"

#include <cstring>
#include <experimental/filesystem>
#include <iostream>
#include <thread>

#include <string>

namespace os {
namespace ocpp {

class OcppManager {
public:	
	OcppManager();
	~OcppManager();

	// void addConnector(cs::connectors::Connector& connector, int time_charge = 0);
	bool init(std::string &ocpp_config_path);
	bool start();
	bool stop();
	bool resetData();
	
	int getChargePointsCount();
	std::vector<std::string> getChargePointIds();

	bool sendGetBaseReport(const std::string &id);
	bool sendVariablesReq(const std::string &id, const std::string& component, const std::string& variable, const std::string& attribute);

	bool sendTriggerStatusNotification(const std::string& id);
	bool sendTriggerMeterValues(const std::string& id, const int evse_id);
	bool sendTriggerHeartbit(const std::string &id);

	// bool reconnect();
	// bool isInit();

private:
	std::unique_ptr<os::ocpp::CentralSystemDemoConfig> m_central_system_config;								// OCPP конфиг зарядной станции
	std::unique_ptr<os::ocpp::DefaultCentralSystemEventsHandler> m_event_handler;								// обработчик событий OCPP
	std::unique_ptr<::ocpp::centralsystem::ocpp20::ICentralSystem20> m_central_system;						// OCPP зарядная станция

	bool m_init;
	bool m_start;

	std::shared_ptr<::ocpp::centralsystem::ocpp20::ICentralSystem20::IChargePoint20> getChargePointById(const std::string& id);
};

} // ocpp
} // os

#endif // OS_OCPP_MANAGER_H

