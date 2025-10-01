#ifndef OS_CENTRAL_SYSTEM_DEMO_CONFIG_H
#define OS_CENTRAL_SYSTEM_DEMO_CONFIG_H

#include "CentralSystemConfig.h"
#include "IniFile.h"

namespace os {
namespace ocpp {

/** @brief Configuration of the Central System demo */
class CentralSystemDemoConfig
{
  public:
    /** @brief Constructor */
    CentralSystemDemoConfig(const std::string& config_file) : m_config(config_file), m_stack_config(m_config) { }

    /** @brief Stack internal configuration */
    ::ocpp::config::ICentralSystemConfig20& stackConfig() { return m_stack_config; }

    /** @brief Boot notification retry interval */
    std::chrono::seconds bootNotificationRetryInterval() const { return m_stack_config.bootNotificationRetryInterval(); }
    /** @brief Heartbeat interval */
    std::chrono::seconds heartbeatInterval() const { return m_stack_config.heartbeatInterval(); }

  private:
    /** @brief Configuration file */
    ::ocpp::helpers::IniFile m_config;

    /** @brief Stack internal configuration */
    os::ocpp::CentralSystemConfig m_stack_config;
};

} // ocpp
} // os

#endif // OS_CENTRAL_SYSTEM_DEMO_CONFIG_H
