#include "OcppManager.h"

#include <iostream>
#include <experimental/filesystem>

namespace os {
namespace ocpp {

OcppManager::OcppManager() {}
OcppManager::~OcppManager() {}

bool OcppManager::init(std::string &ocpp_config_path) {
	if (m_init) {
		std::cout << "OcppManager Error: Already init" << std::endl;
		return false;
	}

	std::experimental::filesystem::path config_path(ocpp_config_path);
	m_central_system_config = std::make_unique<os::ocpp::CentralSystemDemoConfig>(config_path.string());
	m_event_handler = std::make_unique<DefaultCentralSystemEventsHandler>(*m_central_system_config.get());
	m_central_system = ::ocpp::centralsystem::ocpp20::ICentralSystem20::create(m_central_system_config->stackConfig(), *m_event_handler.get());

	m_init = true;
	return true;
}

bool OcppManager::resetData() {
	bool result = false;
	if (m_init) {
		result = m_central_system->resetData();
	}
	return result;
}

bool OcppManager::start() {
	bool result = false;
	if (m_init) {
		result = m_central_system->start();
	}	
	return result;
}

bool OcppManager::stop() {
	bool result = false;
	if (m_init && !m_start) {
		result = m_central_system->stop();
	}
	return result;
}

// bool OcppManager::reconnect() {
// 	bool result = false;
// 	if (m_init) {
// 		result = m_charge_point->reconnect();
// 	}
// 	return result;
// }

// bool OcppManager::isInit() {
// 	return m_init;
// }

} // ocpp
} // os