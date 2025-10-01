// #include "WebServer.h"

#include <iostream>
#include <experimental/filesystem>
#include <memory>
#include <chrono>
#include <thread>

int main(void) {
	// std::cout << "main: Current directory " << std::experimental::filesystem::current_path().c_str() << std::endl;
	// std::string charge_station_config_path = "config/charge_station.ini";
	
	// std::unique_ptr<cs::ChargeStation> station = std::make_unique<cs::ChargeStation>();
	
	// if (!station->initFromConfig(charge_station_config_path)) {
	// 	std::cout << "main: Error parse station config" << std::endl;
	// 	return -1;
	// }
	
	// if (!station->start()) {
	// 	std::cout << "main: Error start station" << std::endl;
	// 	return -1;
	// }

	// cs::web::WebServer server(8080);
    
    // // Простая конфигурация сервера
    // server.setHttpHandler([](const auto& uri, const auto& method, 
    //                        const auto& headers, const auto& body, auto& response) {
    //     response = "<h1>Hello from cs::web::WebServer!</h1>";
    //     return 200;
    // });
    
    // if (server.start()) {
    //     std::cout << "Server started successfully!" << std::endl;
    //     server.run();  // Блокирующий вызов
    // }

	// while(true) {
	// 	std::cout << "********+++ Station +++*********" << std::endl;
	// 	std::cout << "Is ok: 				" << static_cast<int>(station->isOk()) << std::endl;
	// 	std::cout << "Is alarm: 			" << static_cast<int>(station->isAlarm()) << std::endl;
	// 	std::cout << "Is start: 			" << static_cast<int>(station->isStart()) << std::endl;
	// 	std::cout << "Is start modbus:		" << static_cast<int>(station->isStartModbus()) << std::endl;
	// 	std::cout << "Max I: 				" << station->getMaxI() << std::endl;
	// 	std::cout << "Max U: 				" << station->getMaxU() << std::endl;
	// 	std::cout << "Max P: 				" << station->getMaxP() << std::endl;

	// 	std::cout << "Connector count:		" << station->getConnectorCount() << std::endl;

	// 	for (int i = 0; i < station->getConnectorCount(); i++) {
	// 		std::cout << "***++ Connector " << i+1 << "++***" << std::endl;
	// 		std::cout << "Type: " << station->getConnectorType(i) << std::endl;
	// 		std::cout << "I: " << station->getIConnector(i) << std::endl;
	// 		std::cout << "U: " << station->getUConnector(i) << std::endl;
	// 		std::cout << "Is ok: " << static_cast<int>(station->isOkConnector(i)) << std::endl;
	// 		std::cout << "Is in: " << static_cast<int>(station->isInConnector(i)) << std::endl;
	// 		std::cout << "Is charge: " << static_cast<int>(station->isChargeConnector(i)) << std::endl;
	// 		std::cout << "Is alarm: " << static_cast<int>(station->isAlarmConnector(i)) << std::endl;
	// 	} 

	// 	std::this_thread::sleep_for(std::chrono::seconds(5));
	// }

	// std::cout << "main: End of program" << std::endl;

   //  cs::web::WebServer server(8080);
    
    // // Настраиваем обработчик HTTP запросов (опционально)
    // server.setHttpHandler([](const std::string& uri, const std::string& method,
    //                        const std::unordered_map<std::string, std::string>& headers,
    //                        const std::string& body, std::string& response) {
        
    //     std::cout << "HTTP " << method << " " << uri << std::endl;
        
    //     // Для API endpoints можно оставить кастомную логику
    //     if (uri == "/api/custom") {
    //         response = R"({"message": "Custom API endpoint"})";
    //         return 200;
    //     }
        
    //     // Для всех остальных запросов вернем 404, чтобы сервер попробовал статические файлы
    //     return 404;
    // });
    
    // Запускаем сервер
   //  if (server.start()) {
   //      std::thread serverThread([&server]() {
   //          server.run();
   //      });
        
   //      std::cout << "Press Enter to stop server..." << std::endl;
   //      std::cin.get();
        
   //      server.stop();
   //      serverThread.join();
   //  }
    
    return 0;
}
