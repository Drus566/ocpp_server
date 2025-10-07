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

int OcppManager::getChargePointsCount() {
	int result = 0;
	if (m_start) {
		result = m_event_handler->chargePointsCount();
	}
	return result;
}

std::vector<std::string> OcppManager::getChargePointIds() {
	std::vector<std::string> result;
	if (m_start) {
		auto connected_chargepoints = m_event_handler->chargePoints();
		for (auto &iter_chargepoint : connected_chargepoints) {
			auto chargepoint = iter_chargepoint.second->proxy();
			result.push_back(chargepoint->identifier());
		}
	}
	return result;
}

std::shared_ptr<::ocpp::centralsystem::ocpp20::ICentralSystem20::IChargePoint20> OcppManager::getChargePointById(const std::string& id) {
	auto connected_chargepoints = m_event_handler->chargePoints();
	for (auto &iter_chargepoint : connected_chargepoints) {
		auto chargepoint = iter_chargepoint.second->proxy();
		if (chargepoint->identifier() == id) {
			return chargepoint;
		}
	}
	return nullptr;
}

bool OcppManager::sendGetBaseReport(const std::string &id) {
	auto charge_point = getChargePointById(id);

	if (charge_point == nullptr) {
		return false;
	}

	std::string error;
	std::string error_msg;
	::ocpp::messages::ocpp20::GetBaseReportReq get_base_report_req;
	::ocpp::messages::ocpp20::GetBaseReportConf get_base_report_conf;

	get_base_report_req.requestId = std::chrono::system_clock::now().time_since_epoch().count();
	get_base_report_req.reportBase = ::ocpp::types::ocpp20::ReportBaseEnumType::ConfigurationInventory;

	std::cout << "Send get base report..." << std::endl;

	if (!charge_point->call(get_base_report_req, get_base_report_conf, error, error_msg)) {
		std::cout << "Failed : error = " << error << " error_msg = " << error_msg << std::endl;
		return false;
	}

	return true;
}

bool OcppManager::getAllStationInfo(const std::string &id, StationInfo &info) {
	auto charge_point = getChargePointById(id);

	if (charge_point == nullptr) {
		return false;
	}

	return true;
}

bool OcppManager::sendVariablesReq(const std::string &id, const std::string &component, const std::string &variable, const std::string &attribute) {
	auto charge_point = getChargePointById(id);

	if (charge_point == nullptr) {
		return false;
	}

	std::string error;
	std::string error_msg;

	::ocpp::messages::ocpp20::SetVariablesReq set_vars_req;
	::ocpp::messages::ocpp20::SetVariablesConf set_vars_conf;
	::ocpp::types::ocpp20::SetVariableDataType var;

	// var.variable.name.assign("HeartbeatInterval");
	// var.component.name.assign("OCPPCommCtrlr");
	// var.attributeValue.assign("10");

	var.variable.name.assign(variable);
	var.component.name.assign(component);
	var.attributeValue.assign(attribute);

	set_vars_req.setVariableData.push_back(std::move(var));

	std::cout << "Send variables req..." << std::endl;

	if (charge_point->call(set_vars_req, set_vars_conf, error, error_msg)) {
		std::cout << "Done!" << std::endl;
	}
	else {
		std::cout << "Failed : error = " << error << " error_msg = " << error_msg << std::endl;
		return false;
	}

	return true;
}

bool OcppManager::sendTriggerStatusNotification(const std::string& id) {
	auto charge_point = getChargePointById(id);
	
	if (charge_point == nullptr) {
		return true;
	}

	std::string error;
	std::string error_msg;
	::ocpp::messages::ocpp20::TriggerMessageReq trigger_msg_req;
	::ocpp::messages::ocpp20::TriggerMessageConf trigger_msg_conf;

	trigger_msg_req.requestedMessage = ::ocpp::types::ocpp20::MessageTriggerEnumType::StatusNotification;

	std::cout << "Trigger status notification..." << std::endl;

	if (charge_point->call(trigger_msg_req, trigger_msg_conf, error, error_msg)) {
		std::cout << "Done!" << std::endl;
	}
	else {
		std::cout << "Failed : error = " << error << " error_msg = " << error_msg << std::endl;
		return false;
	}
	return true;
}

bool OcppManager::sendTriggerMeterValues(const std::string& id, const int evse_id) {
	auto charge_point = getChargePointById(id);
	
	if (charge_point == nullptr) {
		return true;
	}

	std::string error;
	std::string error_msg;
	::ocpp::messages::ocpp20::TriggerMessageReq trigger_msg_req;
	::ocpp::messages::ocpp20::TriggerMessageConf trigger_msg_conf;

	trigger_msg_req.evse.value().id = evse_id;
	trigger_msg_req.requestedMessage = ::ocpp::types::ocpp20::MessageTriggerEnumType::MeterValues;

	std::cout << "Trigger meter values..." << std::endl;

	if (charge_point->call(trigger_msg_req, trigger_msg_conf, error, error_msg)) {
		std::cout << "Done!" << std::endl;
	}
	else {
		std::cout << "Failed : error = " << error << " error_msg = " << error_msg << std::endl;
		return false;
	}
	return true;
}

bool OcppManager::sendTriggerHeartbit(const std::string& id) {
	auto charge_point = getChargePointById(id);
	
	if (charge_point == nullptr) {
		return true;
	}

	std::string error;
	std::string error_msg;
	::ocpp::messages::ocpp20::TriggerMessageReq trigger_msg_req;
	::ocpp::messages::ocpp20::TriggerMessageConf trigger_msg_conf;

	trigger_msg_req.evse.clear();
	trigger_msg_req.requestedMessage = ::ocpp::types::ocpp20::MessageTriggerEnumType::Heartbeat;

	std::cout << "Trigger heartbeat..." << std::endl;

	if (charge_point->call(trigger_msg_req, trigger_msg_conf, error, error_msg)) {
		std::cout << "Done!" << std::endl;
	}
	else {
		std::cout << "Failed : error = " << error << " error_msg = " << error_msg << std::endl;
		return false;
	}
	return true;
}

} // ocpp
} // os