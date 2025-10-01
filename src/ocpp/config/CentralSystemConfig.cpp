#include "CentralSystemConfig.h"

namespace os {
namespace server {

/** @brief Section name for the parameters */
static const std::string STACK_PARAMS = "CentralSystem";

CentralSystemConfig::CentralSystemConfig(::ocpp::helpers::IniFile &config) : m_config(config) {}

std::string CentralSystemConfig::databasePath() const { 
	return getString("DatabasePath"); 
}

std::string CentralSystemConfig::jsonSchemasPath() const { 
	return getString("JsonSchemasPath"); 
}

std::string CentralSystemConfig::listenUrl() const { 
	return getString("ListenUrl"); 
}

std::chrono::milliseconds CentralSystemConfig::callRequestTimeout() const {
	return get<std::chrono::milliseconds>("CallRequestTimeout"); 
}

std::chrono::seconds CentralSystemConfig::webSocketPingInterval() const { 
	return get<std::chrono::seconds>("WebSocketPingInterval"); 
}

bool CentralSystemConfig::httpBasicAuthent() const {
	return getBool("HttpBasicAuthent");
}

std::string CentralSystemConfig::tlsv12CipherList() const {
	return getString("Tlsv12CipherList");
}

std::string CentralSystemConfig::tlsv13CipherList() const {
	return getString("Tlsv13CipherList");
}

std::string CentralSystemConfig::tlsEcdhCurve() const {
	return getString("TlsEcdhCurve");
}

std::string CentralSystemConfig::tlsServerCertificate() const {
	return getString("TlsServerCertificate");
}

std::string CentralSystemConfig::tlsServerCertificatePrivateKey() const {
	return getString("TlsServerCertificatePrivateKey");
}

std::string CentralSystemConfig::tlsServerCertificatePrivateKeyPassphrase() const {
	return getString("TlsServerCertificatePrivateKeyPassphrase");
}

std::string CentralSystemConfig::tlsServerCertificateCa() const {
	return getString("TlsServerCertificateCa");
}

bool CentralSystemConfig::tlsClientCertificateAuthent() const {
	return getBool("TlsClientCertificateAuthent");
}

unsigned int CentralSystemConfig::logMaxEntriesCount() const {
	return get<unsigned int>("LogMaxEntriesCount");
}

unsigned int CentralSystemConfig::incomingRequestsFromCpThreadPoolSize() const {
	return get<unsigned int>("IncomingRequestsFromCpThreadPoolSize");
};

std::chrono::seconds CentralSystemConfig::bootNotificationRetryInterval() const { 
	return get<std::chrono::seconds>("BootNotificationRetryInterval"); 
}

std::chrono::seconds CentralSystemConfig::heartbeatInterval() const { 
	return get<std::chrono::seconds>("HeartbeatInterval"); 
}

} // server
} // os