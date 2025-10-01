#ifndef OS_H
#define CS_H

#include <string>
#include <memory>
#include <mutex>

namespace os {
namespace server {

class Server {

public:
	Server();
	~Server();

	/* Парсинг конфига зарядной станции */
	// bool initFromConfig(std::string &charge_station_config_path);

	/****** СТАНЦИЯ *******/

	/* Запущена ли станция */
	// bool isStart();
	// /* Старт логики зарядной станции */
	// bool start();
	// /* Стоп логики зарядной станции */
	// bool stop();

	/* Получить статус аварии станции */
	// bool isAlarm();
	// /* Получить статус досутпен ли коннектор */
	// bool isOk();

	/* Получить максимальное напряжение станции */
	// float getMaxP();
	// /* Получить максимальное напряжение станции */
	// float getMaxU();
	// /* Получить максимальный ток станции */
	// float getMaxI();

	/* Установить максимальное напряжение станции */
	// void setMaxU(int& val);
	// /* Установить максимальный ток станции */
	// void setMaxI(int& val);

	/* Установить статус аварии станции */
	// void setAlarm(bool flag);
	// /* Установить статус доступности станции */
	// void setOk(bool flag);


	/****** КОННЕКТОР *******/

	/* Получить строку типа коннектора */
	// std::string getConnectorType(int connector_id);
	// /* Получить количество коннекторов */
	// int getConnectorCount();
	// /* Добавление коннектора */
	// void addConnector(int connector_id, cs::types::ConnectorType connector_type, int quantity = 0);
	// /* Удаление коннектора по id */
	// void removeConnector(int connector_id);
	// /* Удаление всех коннекторов заданного типа */
	// void removeConnector(cs::types::ConnectorType connector_type);

	// /* Запуск заряда на коннекторе */
	// void startChargeConnector(int connector_id);
	// /* Останов заряда на коннекторе */
	// void stopChargeConnector(int connector_id);

	// /* Получить текущее значение напряжения коннектора */
	// float getUConnector(int connector_id);
	// /* Получить текущее значение ток коннектора */
	// float getIConnector(int connector_id);

	// /* Получить статус аварии коннектора */
	// bool isAlarmConnector(int connector_id);
	// /* Получить статус заряда коннектора */
	// bool isChargeConnector(int connector_id);
	// /* Получить статус вставлен ли коннектор */
	// bool isInConnector(int connector_id);
	// /* Получить статус досутпен ли коннектор */
	// bool isOkConnector(int connector_id);

	// /* Установить текущее значение напряжения коннектора */
	// void setUConnector(int connector_id, float &val);
	// /* Установить текущее значение тока коннектора */
	// void setIConnector(int connector_id, float &val);

	// /* Установить статус аварии коннектора */
	// bool setAlarmConnector(int connector_id, bool flag);

	// /* Установить статус заряда коннектора */
	// bool setChargeConnector(int connector_id, bool flag);
	// /* Установить флаг заряда коннектора(ов) по типу коннектора(ов) */
	// bool setChargeConnector(cs::types::ConnectorType type, bool status);
	// /* Установить статус вставлен ли коннектор */
	// bool setInConnector(int connector_id, bool flag);
	// /* Установить статус доступности коннектора */
	// bool setOkConnector(int connector_id, bool flag);

	// /* Действует ли модбас */
	// bool isStartModbus();
	// /* Установить старт/стоп модбас */
	// bool setStatusModbus(bool flag);

private:
	bool m_start; 									// сервер в работе

	// int m_id;										// id станции
	// int m_connector_count;						// количество коннекторов

	// std::string m_description;					// описание станции
	// std::string m_serial_number;				// серийный номер станции

	// std::string m_config_path; 				// путь к конфигу
	// std::string m_ocpp_config_path;			// путь к ocpp конфигу
	// std::string m_ocpp_model_path;			// путь к ocpp модели
	// std::string m_modbus_config_path;		// путь к modbus конфигу

	// bool m_modbus; 								// используется ли модуль модбаса 
	// bool m_ocpp;									// используется ли модуль ocpp

	// // bool m_init; 								// инициализирована ли станция

	// std::unique_ptr<cs::config::Config> m_config;									// конфигурация зарядной станции
	// std::unique_ptr<cs::connectors::ConnectorsManager> m_connector_manager;	// управление коннекторами

	// std::unique_ptr<cs::modbus::ModbusMaster> m_modbus_master;	// указатель на модбас мастер
	// std::unique_ptr<cs::data::ChargeStationData> m_data;			// указатель на данные станции

	// std::unique_ptr<cs::ocpp::OcppManager> m_ocpp_manager;			// указатель на OCPP менеджер 	

	// std::mutex m_connector_mutex;

	// /* Инициализация OCPP клиента */
	// bool initOcpp();
	// /* Инициализация модбас данных */
	// bool initModbus();
	// /* Инициализация коннекторов из конфига */
	// bool initConnectors();
};

} // os
} // server

#endif // OS