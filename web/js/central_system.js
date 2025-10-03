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

	// === RPC МЕТОДЫ ДЛЯ СТАНЦИИ === 

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
		}
		catch (error) {
			this.handleRpcError('Ошибка получения информации о станции', error);
		}
	}

	async getConfiguration() {
		if (!this.validateCurrentStation()) return;

		try {
			const response = await this.rpc_client.call('GetConfiguration', {
				stationId: this.current_station
			});
			this.logSuccess('Конфигурация получена');
			this.log(`Конфигурация: ${JSON.stringify(this.parseResponse(response))}`);
		}
		catch (error) {
			this.handleRpcError('Ошибка получения конфигурации', error);
		}
	}

	async getVariable() {
		if (!this.validateCurrentStation()) return;

		try {
			const response = await this.rpc_client.call('GetVariable', {
				stationId: this.current_station
			});
			this.logSuccess('Переменные получены');
			this.log(`Переменные: ${JSON.stringify(this.parseResponse(response))}`);
		}
		catch (error) {
			this.handleRpcError('Ошибка получения переменных', error);
		}
	}

	async setVariable() {
		if (!this.validateCurrentStation()) return;

		const component = document.querySelector('#station_control__set_variable_component');
		const variable = document.querySelector('#station_control__set_variable_variable');
		const attribute = document.querySelector('#station_control__set_variable_attribute');

		if (!component || !variable) {
			this.logError('Заполните обязательные поля: Component и Variable');
			return;
		}

		try {
			await this.rpc_client.call('SetVariable', {
				stationId: this.current_station,
				component,
				variable,
				attribute: attribute || undefined
			});
			this.logSuccess('Переменная успешно записана');
		}
		catch (error) {
			this.handleRpcError('Ошибка записи переменной', error);
		}
	}

	async rebootStation() {
		if (!this.validateCurrentStation()) return;

		try {
			await this.rpc_client.call('RebootStation', {
				stationId: this.current_station
			});
			this.logSuccess('Команда перезагрузки отправлена');
		}
		catch (error) {
			this.handleRpcError('Ошибка перезагрузки станции', error);
		}
	}

	async clearCache() {
		if (!this.validateCurrentStation()) return;

		try {
			await this.rpc_client.call('ClearCache',  {
				stationId: this.current_station
			});
			this.logSuccess('Кэш успешно очищен');
		}
		catch (error) {
			this.handleRpcError('Ошибка очистки кэша', error);
		}
	}

	// === RPC МЕТОДЫ КОННЕКТОРА ===
	
	async requestStartTransaction() {
		if (!this.validateCurrentStation()) return;

		try {
			await this.rpc_client.call('RequestStartTransaction', {
				stationId: this.current_station,
				connectorId: this.current_connector
			});
			this.logSuccess('Запрос на начало транзакции отправлен');
		}
		catch (error) {
			this.handleRpcError('Ошибка начала транзакции', error);
		}
	}

	async requestStopTransaction() {
		if (!this.validateCurrentStation()) return;

		try {
			await this.rpc_client.call('RequestStopTransaction', {
				stationId: this.current_station,
				connectorId: this.current_connector
			});
			this.logSuccess('Запрос на остановку транзакции отправлен');
		}
		catch (error) {
			this.handleRpcError('Ошибка остановки транзакции', error);
		}
	}

	async unlockConnector() {
		if (!this.validateCurrentStation()) return;

		try {
			await this.rpc_client.call('UnlockConnector', {
				stationId: this.current_station,
				connectorId: this.current_connector
			});
			this.logSuccess('Команда разблокировки коннектора отправлена');
		}
		catch (errro) {
			this.handleRpcError('Ошибка разблокировки коннектора', error);
		}
	}

	// === TRIGGER МЕТОДЫ ===

	async triggerMeterValues() {
		if (!this.validateCurrentStation()) return;

		const evseIdInput = document.querySelector('#station_control__trigger_meter_values');
		const evseId = evseIdInput.value || this.current_connector;

		try {
			await this.rpc_client.call('TriggerMeterValues', {
				stastionId: this.current_station,
				evseId
			});
			this.logSuccess(`Meter values triggered for EVSE: ${evseId}`);
		}
		catch (error) {
			this.handleRpcError('Ошибка trigger MeterValues', error);
		}
	}

	async triggerStatusNotification() {
		if (!this.validateCurrentStation()) return;

		try {
			await this.rpc_client.call('TriggerStatusNotification', {
				stationId: this.current_station
			});
			this.logSuccess('Status notification triggered');
		}
		catch (error) {
			this.handleRpcError('Ошибка trigger StatusNotification', error);
		}
	}

	async triggerHeartbeat() {
		if (!this.validateCurrentStation()) return;

		try {
			await this.rpc_client.call('TriggerHeartbeat', {
				stationId: this.current_station
			});
			this.logSuccess('Heartbeat triggered');
		}
		catch (error) {
			this.handleRpcError('Ошибка trigger Heartbeat', error);
		}
	}

	// === ОБРАБОТЧИКИ RPC УВЕДОМЛЕНИЙ ===

	handleStatusNotification(data) {
		this.log(`StatusNotification: ${JSON.stringify(data)}`);

		if (data.stationId === this.current_station) {
			this.updateStationStatus(data.status);

			if (data.connector_id === this.connector_id) {
				this.updateConnectorStatus(data.connectorStatus);
			}
		}
	}

	handleMeterValues(data) {
		this.log(`MeterValues: ${JSON.stringify(data)}`);

		if (data.stationId === this.current_station && data.connectorId == this.current_connector) {
			this.updateMeterValues(data.values);
		}
	}

	handleHeartbeat(data) {
		this.log(`Heartbeat received from station: ${data.stationId}`);
	}

	// === ОБНОВЛЕНИЕ UI ===

	updateStationDropdown(stations) {
		const select = document.getElementById('station-select');
		select.innerHTML = '';

		stations.forEach(station => {
			const option = document.createElement('option');
			option.value = station.id;
			option.textContent = station.name || `Станция ${station.id}`;
			select.appendChild(option);
		});
	}

	updateConnectorsDropdown(connectors) {
		const select = document.getElementById('connector-select');
		select.innerHTML = '';

		if (connectors && connectors.length > 0) {
			connectors.forEach(connector => {
				const option = document.createElement('option');
				option.value = connector.id;
				option.textContent = `Коннектор ${connector.id}`;
				select.appendChild(option);
			});
		}
	}

	updateStationInfo(station_data) {
		// Обновление статуса
		// const status_element = document.querySelector('#station__status');
		// if (status_element) {
		// 	status_element.textContent = this.getStatusText(station_data.status);
		// 	status_element.className = `info-item__value status--${station_data.status}`;
		// }
		this.updateStationStatus(station_data.status);

		// Обновление количества коннекторов
		const connectors_count_element = document.querySelector('#station__connectors_count');
		if (connectors_count_element && station_data.connectors) {
			connectors_count_element.textContent = station_data.connectors.length;
		}

		// Обновление максимальное мощности
		const power_element = document.querySelector('#station__max_power');
		if (power_element && station_data.maxPower) {
			power_element.textContent = `${station_data.maxPower} кВт`;
		}
	}

	updateConnectorInfo(connector_data) {
		// Тип
		const connector_type = document.querySelector('#connector__type');
		connector_type.textContent = connector_data.type;

		// Статус 
		// const connector_status = document.querySelector('#connector__status');
		// connector_status.textContent = connector_data.status;
		// connector_status.className = `info-item__value status-indicator status-indicator--${connector_data.status}`;
		this.updateConnectorStatus(connector_data.status);

		// Счетчик
		if (connector_data.meter != undefined) {
			updateMeterValues(connector_data.meter);
			// const connector_meter = document.querySelector('#connector__meter');
			// connector_meter.textContent = `${connector_data.meter} кВт*ч`;
		}

		// Мощность
		if (connector_data.power !== undefined) {
			updatePower(connector_data.power());
			// const connector_power = document.querySelector('#connector__power');
			// connector_power.textContent = `${connector_data.power} кВт`;
		}
	}

	updateStationStatus(status) {
		const status_element = document.querySelector('#station__status');
		if (status_element) {
			status_element.textContent = this.getStatusText(status);
			status_element.className = `info-item__value status--${status}`;
		}
	}

	updateConnectorStatus(status) {
		const connector_status = document.querySelector('#connector__status');
		connector_status.textContent = status;
		connector_status.className = `info-item__value status-indicator status-indicator--${status}`;
	}

	updateMeterValues(meter) {
		const connector_meter = document.querySelector('#connector__meter');
		connector_meter.textContent = `${meter} кВт*ч`;
	}

	updatePower(power) {
		const connector_power = document.querySelector('#connector__power');
		connector_power.textContent = `${power} кВт`;
	}

	// === ВСПОМОГАТЕЛЬНЫЕ МЕТОДЫ ===

	validateCurrentStation() {
		if (!this.current_station) {
			this.logError('Сначала выберите станцию');
			return false;
		}
		return true;
	}

	validateCurrentConnector() {
		if (!this.current_connector) {
			this.logError('Сначала выберите коннектор');
			return false;
		}
		return true;
	}

	parseResponse(response) {
		// Обработка вложенного JSON, если необходимо
		if (response && response.result) {
			if (typeof response.result === 'string') {
				try {
					return JSON.parse(response.result);
				} catch (e) {
					return response.result;
				}
			}
			return response.result;
		}
		return response;
	}

	cacheStationsData(stations) {
		this.stations.clear();
		stations.forEach(station => {
			this.stations.set(station.id, station);
		});
	}

	getStatusText(status) {
		const statusMap = {
			'working': 'В работе',
			'alarm': 'Авария',
			'disabled': 'Отключена',
			'': 'Не определно'
		};
		return statusMap[status] || status;
	}

	// === ЛОГИРОВАНИЕ ===

	log(message) {
		this.addLogEntry(message, 'info');
		console.log(`[CentralSystem] ${message}`);
	}

	logSuccess(message) {
		this.addLogEntry(message, 'success');
		console.log(`[CentralSystem] ✅ ${message}`);
	}

	logError(message) {
		this.addLogEntry(message, 'error');
		console.log(`[CentralSystem] ❌ ${message}`);
	}

	addLogEntry(message, type = 'info') {
		const log_container = document.querySelector('.log-container');
		const log_entry = document.createElement('div');
		log_entry.className = `log-entry log-entry--${type}`;

		const time = new Date().toLocaleTimeString();
		log_entry.innerHTML = `
			<span class="log-entry__time">[${time}]</span>
			<span class="log-entry__message">${message}</span>
		`;

		log_container.appendChild(log_entry);
		log_container.scrollTop = log_container.scrollHeight;
	}

	handleRpcError(user_message, error) {
		const error_message = error.message || 'Неизвестная ошибка';
		this.logError(`${user_message}: ${error_message}`);
	}
}

// Инициализация при загрузке страницы 
document.addEventListener('DOMContentLoaded', () => {
	window.central_system = new CentralSystem(window.rpc_client);
});