class CentralSystem {
	constructor(rpc_client) {
		this.rpc_client = rpc_client;
		this.current_station = null;
		this.current_connector = null;
		this.stations = new Map();
		this.is_initialized = false;

		this.init();
	}

	init() {
		if (this.is_initialized) return;

		this.bindEvents();
		this.setupRpcHandlers();
		this.loadInitialData();

		this.is_initialized = true;
		console.log("CentralSystem initialized");
	}

	/** Привязка DOM событиый */
	bindEvents() {
		// Выбор станции
		document.getElementById('station-select').addEventListener('change', (e) => { 
			this.handleStationChange(e.target.value); 
		});

		// Кнопки управления станцией
		this.bindStationControls();

		// Кнопки управления коннектором
		this.bindConnectorControls();

		// Trigger кнопки
		this.bindTriggerControls();
	}

	/** Настройка обработчиков RPC событий */
	setupRpcHandlers() {
		this.rpc_client.on('connected', () => {
			this.updateConnectionStatus(true);
			this.loadStationsList();
		});

		this.rpc_client.on('disconnected', () => {
			this.updateConnectionStatus(false);
		});

		this.rpc_client.on('StatusNotification', (data) => {
			this.handleStatusNotification(data);
		});

		this.rpc_client.on('MeterValues', (data) => {
			this.handleMeterValues(data);
		});

		this.rpc_client.on('Heartbeat', (data) => {
			this.handleHeartbeat(data);
		});
	}

	/** Загрузка начальных данных */
	async loadInitialData() {
		try {
			// Загрузка списка станций при старте
			if (this.rpc_client.connected) {
				await this.loadStationsList();
			}
		} 
		catch (error) {
			console.error('Error loading initial data:', error);
			this.logError('Ошибка загрузки начальных данных');
		}
	}

	/** Загрузка списка станций */
	async loadStationsList() {
		try {
			this.log('Загрузка списка станций...');

			const response = await this.rpc_client.call('GetStations');
			const stations = this.parseResponse(response);

			this.updateStationsDropdown(stations);
			this.cacheStationsData(stations);

			// Автоматически выбирам первую станцию
			// if (stations.length > 0 && !this.current_station) {
			// 	this.handleStationChange(stations[0].id);
			// }
		}
		catch (error) {
			console.log('Error loading stations list:', error);
			this.logError('Ошибка загрузки списка станций');
		}
	}

	/** Обработчик изменений выбранной станции */
	async handleStationChange(station_id) {
		try {
			this.log(`Выбор станции: ${station_id}`);

			const response = await this.rpc_client.call('GetStationStatus', { station_id });
			const station_data = this.parseResponse(response);

			this.current_station = station_id;
			this.updateStationInfo(station_data);
			this.updateConnectorsDropdown(station_data.connectors);

			// Выбираем первый коннектор по умолчанию
			if (station_data.connectors && station_data.connectors.length > 0) {
				this.handleConnectorChange(station_data.connectors[0].id);
			}
		}
		catch (error) {
			console.log('Error changing station:', error);
			this.logError(`Ошибка загрузки данных станций ${station_id}`);
		}
	}

	/** Обработчик изменения выбранного коннектора ? */
	async handleConnectorChange(connector_id) {
		try {
			this.log(`Выбор коннектора: ${connector_id}`);

			const response = await this.rpc_client.call('GetConnectorStatus', {
				stationId: this.current_station,
				connector_id
			});
			
			const connector_data = this.parseResponse(response);
			this.current_connector = connector_id;
			this.updateConnectorInfo(connector_data);
		}
		catch (error) {
			console.error('Error changing connector:', error);
			this.logError(`Ошибка загрузки данных коннектора ${connector_id}`);
		}
	}

	/** Привязка обработчиков для кнопок управления станцией */
	bindStationControls() {
		// Отключить станцию
		document.querySelector('#station_control__disconnect').addEventListener('click', () => {
			this.disconnectStation();
		});

		// Получить конфигурацию
		document.querySelector('#station_control__boot_notification').addEventListener('click', () => {
			this.getStationInfo();
		});

		// Получить конфигурацию
		document.querySelector('#station_control__get_configuration').addEventListener('click', () => {
			this.getConfiguration();
		});

		// Получить переменную
		document.querySelector('#station_control__get_variable').addEventListener('click', () => {
			this.getVariable();
		});

		// Записать переменную
		document.querySelector('#station_control__set_variable').addEventListener('click', () => {
			this.setVariable();
		});

		// Перезагрузка
		document.querySelector('#station_control__reset').addEventListener('click', () => {
			this.rebootStation();
		});

		// Очистить кэш
		document.querySelector('#station_control__clear_cache').addEventListener('click', () => {
			this.clearCache();
		});
	}

	/** Привязка обработчиков для кнопок управления коннектором */
	bindConnectorControls() {
		document.querySelector('#connector_control__request_start_transaction').addEventListener('click', () => {
			this.requestStartTransaction();
		});

		document.querySelector('#connector_control__request_stop_transaction').addEventListener('click', () => {
			this.requestStopTransaction();
		});

		document.querySelector('#connector_control__unlock_connector').addEventListener('click', () => {
			this.unlockConnector();
		});
	}

	/** Привязка обработчиков для Trigger кнопок */
	bindTriggerControls() {
		document.querySelector('#station_control__trigger_meter_values').addEventListener('click', () => {
			this.triggerMeterValues();
		});

		document.querySelector('#station_control__trigger_status_notification').addEventListener('click', () => {
			this.triggerStatusNotification();
		});

		document.querySelector('#station_control__trigger_heartbeat').addEventListener('click', () => {
			this.triggerHeartbeat();
		});
	}

	// === RPC методы для станции ===
	
	async disconnectStation() {
		if (!this.validateCurrentStation()) return;

		try {
			this.log(`Отключение станции ${this.current_station}`);
			await this.rpc_client.call('DisconnectStation', {
				stationId: this.current_station
			});
			this.logSuccess('Станция успешно отключена');
		}
		catch (error) {
			this.handleRpcError('Ошибка отключения станции', error);
		}
	}

	async getStationInfo() {
		if (!this.validateCurrentStation()) return;

		try {
			const response = await this.rpc_client.call('GetStationInfo', {
				stationId: this.current_station
			});
			this.logSuccess('Информация о станции получена');
			this.log(`Информация о станции: ${JSON.stringify(this.parseResponse(response))}`);
		}
		catch (error) {
			this.handleRpcError('Ошибка получения информации о станции', error);
		}
	}
}