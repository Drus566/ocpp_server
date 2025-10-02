class OCPPSystem {
	constructor() {
		this.stations = {};
		this.selectedStationId = null;
		this.selectedConnectorId = null;
		this.connectionState = 'disconnected';
		this.init();
	}

	init() {
		console.log('Initializing OCPP System...');

		// Инициализация RPC клиента
		if (typeof rpcClient !== 'undefined') {
			this.setupRpcHandlers();
		} else {
			console.warn('RpcClient not found, using demo mode');
			this.initDemoMode();
		}

		this.setupEventHandlers();
		this.addLog("Система OCPP 2.0.1 инициализирована");
	}

	setupRpcHandlers() {
		rpcClient.on('connected', () => {
			console.log('RPC client connected successfully');
			this.updateConnectionStatus(true);
			this.connectionState = 'connected';
			this.loadStations();
		});

		rpcClient.on('disconnected', () => {
			console.log('RPC client disconnected');
			this.updateConnectionStatus(false);
			this.connectionState = 'disconnected';
		});

		rpcClient.on('error', (error) => {
			console.error('RPC client error:', error);
			this.addLog('Connection error: ' + error.message);
		});

		// OCPP-specific events
		rpcClient.on('statusNotification', (params) => {
			console.log('Status notification:', params);
			this.updateConnectorStatus(params.chargePointId, params.connectorId, params.status);
		});

		rpcClient.on('meterValues', (params) => {
			console.log('Meter values:', params);
			this.updateConnectorMeterValues(params.chargePointId, params.connectorId, params.meterValue);
		});

		rpcClient.on('heartbeat', (params) => {
			console.log('Heartbeat received:', params);
			this.updateStationStatus(params.chargePointId, 'connected');
		});

		// Подключаемся к серверу
		rpcClient.connect();
	}

	initDemoMode() {
		console.log('Starting in demo mode');
		// Используем демо данные из предыдущей версии
		this.stations = {
			"CS001": {
				name: "Станция 001",
				status: "connected",
				connectorCount: 4,
				maxPower: 150,
				connectors: {
					1: { type: "GBT", status: "ok", meter: 1250, power: 0 },
					2: { type: "CCS2", status: "charge", meter: 3200, power: 50 },
					3: { type: "TYPE2", status: "unavailable", meter: 0, power: 0 },
					4: { type: "CHADEMO", status: "alarm-ev", meter: 0, power: 0 }
				}
			},
			"CS002": {
				name: "Станция 002",
				status: "disconnected",
				connectorCount: 2,
				maxPower: 100,
				connectors: {
					1: { type: "TYPE2", status: "ok", meter: 500, power: 0 },
					2: { type: "CCS2", status: "ok", meter: 800, power: 0 }
				}
			},
			"CS003": {
				name: "Станция 003",
				status: "error",
				connectorCount: 3,
				maxPower: 120,
				connectors: {
					1: { type: "GBT", status: "alarm-cp", meter: 0, power: 0 },
					2: { type: "TYPE2", status: "in", meter: 1500, power: 22 },
					3: { type: "CHADEMO", status: "charge", meter: 2100, power: 45 }
				}
			}
		};

		this.updateConnectionStatus(true);
		this.loadStations();
	}

	async loadStations() {
		try {
			if (this.connectionState === 'connected') {
				// Получаем количество станций
				const countResponse = await rpcClient.call('getChargePointsCount');
				const count = this.parseResponse(countResponse);

				// Получаем ID станций
				const idsResponse = await rpcClient.call('getChargePointIds');
				const stationIds = this.parseResponse(idsResponse);

				console.log('Loaded stations:', stationIds);

				// Загружаем информацию о каждой станции
				for (const stationId of stationIds) {
					await this.loadStationInfo(stationId);
				}
			} else {
				// Демо режим - используем существующие станции
				this.updateStationsList();
			}
		} catch (error) {
			console.error('Error loading stations:', error);
			this.addLog('Error loading stations: ' + error.message);
			// В случае ошибки используем демо данные
			this.updateStationsList();
		}
	}

	async loadStationInfo(stationId) {
		try {
			// Здесь можно добавить вызовы для получения детальной информации о станции
			this.stations[stationId] = {
				name: stationId,
				status: "connected",
				connectorCount: 2, // По умолчанию
				maxPower: 150, // По умолчанию
				connectors: {
					1: { type: "TYPE2", status: "available", meter: 0, power: 0 },
					2: { type: "CCS2", status: "available", meter: 0, power: 0 }
				}
			};

			this.updateStationsList();
		} catch (error) {
			console.error(`Error loading station ${stationId}:`, error);
		}
	}

	parseResponse(response) {
		if (typeof response === 'string') {
			try {
				return JSON.parse(response);
			} catch (e) {
				return response;
			}
		} else if (response && response.result) {
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

	updateStationsList() {
		const stationSelect = document.getElementById('station-select');
		stationSelect.innerHTML = '<option value="">-- Выберите станцию --</option>';

		for (const stationId in this.stations) {
			const station = this.stations[stationId];
			const option = document.createElement('option');
			option.value = stationId;
			option.textContent = `${stationId} - ${station.name}`;
			stationSelect.appendChild(option);
		}

		this.addLog(`Загружено станций: ${Object.keys(this.stations).length}`);
	}

	updateConnectionStatus(connected) {
		// Можно добавить индикатор статуса соединения в интерфейс
		const status = connected ? 'Connected' : 'Disconnected';
		this.addLog(`Статус соединения: ${status}`);
	}

	// Обработчики событий UI
	setupEventHandlers() {
		// Выбор станции
		document.getElementById('station-select').addEventListener('change', (e) => {
			this.handleStationSelect(e.target.value);
		});

		// Выбор коннектора
		document.getElementById('connector-select').addEventListener('change', (e) => {
			this.handleConnectorSelect(e.target.value);
		});

		// Кнопки управления станцией
		document.getElementById('disconnect-btn').addEventListener('click', () => {
			this.handleDisconnect();
		});

		document.getElementById('get-report-btn').addEventListener('click', () => {
			this.handleGetBaseReport();
		});

		// Кнопки действий OCPP
		document.getElementById('send-variables-btn').addEventListener('click', () => {
			this.handleSendVariables();
		});

		document.getElementById('send-status-btn').addEventListener('click', () => {
			this.handleSendStatusNotification();
		});

		document.getElementById('send-meter-btn').addEventListener('click', () => {
			this.handleSendMeterValues();
		});

		document.getElementById('send-heartbit-btn').addEventListener('click', () => {
			this.handleSendHeartbit();
		});

		// Очистка лога
		document.getElementById('clear-log-btn').addEventListener('click', () => {
			this.clearLog();
		});
	}

	handleStationSelect(stationId) {
		if (!stationId) {
			document.getElementById('station-info').style.display = 'none';
			document.getElementById('connector-selector').style.display = 'none';
			document.getElementById('connector-info').style.display = 'none';
			return;
		}

		this.selectedStationId = stationId;
		const station = this.stations[stationId];

		// Обновляем информацию о станции
		document.getElementById('station-id').textContent = `${stationId} - ${station.name}`;
		document.getElementById('connector-count').textContent = `Connector count: ${station.connectorCount}`;
		document.getElementById('max-power').textContent = `Максимальная мощность: ${station.maxPower} kW`;

		// Обновляем статус и иконку станции
		this.updateStationDisplay(stationId, station.status);

		// Показываем информацию о станции
		document.getElementById('station-info').style.display = 'flex';

		// Заполняем выпадающий список коннекторов
		this.updateConnectorsList(stationId);

		// Показываем селектор коннекторов
		document.getElementById('connector-selector').style.display = 'block';

		this.addLog(`Выбрана станция: ${stationId}`);
	}

	updateStationDisplay(stationId, status) {
		const stationIcon = document.getElementById('station-icon');
		const stationStatus = document.getElementById('station-status');

		let statusText, statusColor;
		switch (status) {
			case 'connected':
				statusText = 'Connected';
				statusColor = '#27ae60'; // зеленый
				break;
			case 'disconnected':
				statusText = 'Disconnected';
				statusColor = '#95a5a6'; // серый
				break;
			case 'error':
				statusText = 'Error';
				statusColor = '#e74c3c'; // красный
				break;
			case 'charging':
				statusText = 'Charging';
				statusColor = '#3498db'; // синий
				break;
			default:
				statusText = 'Unknown';
				statusColor = '#95a5a6'; // серый
		}

		stationStatus.textContent = statusText;
		stationIcon.style.backgroundColor = statusColor;
	}

	updateConnectorsList(stationId) {
		const connectorSelect = document.getElementById('connector-select');
		connectorSelect.innerHTML = '<option value="">-- Выберите коннектор --</option>';

		const station = this.stations[stationId];
		for (const connectorId in station.connectors) {
			const option = document.createElement('option');
			option.value = connectorId;
			option.textContent = `Коннектор ${connectorId}`;
			connectorSelect.appendChild(option);
		}
	}

	handleConnectorSelect(connectorId) {
		if (!connectorId) {
			document.getElementById('connector-info').style.display = 'none';
			return;
		}

		this.selectedConnectorId = connectorId;
		const station = this.stations[this.selectedStationId];
		const connector = station.connectors[connectorId];

		// Обновляем информацию о коннекторе
		document.getElementById('connector-type').textContent = connector.type;
		document.getElementById('connector-meter').textContent = connector.meter;
		document.getElementById('connector-power').textContent = connector.power;

		// Обновляем статус коннектора
		this.updateConnectorStatusDisplay(connector.status);

		// Показываем информацию о коннекторе
		document.getElementById('connector-info').style.display = 'block';

		this.addLog(`Выбран коннектор ${connectorId} на станции ${this.selectedStationId}`);
	}

	updateConnectorStatusDisplay(status) {
		const connectorStatus = document.getElementById('connector-status');

		let statusText, statusClass;
		switch (status) {
			case 'alarm-ev':
				statusText = 'Alarm EV';
				statusClass = 'status-alarm-ev';
				break;
			case 'alarm-cp':
				statusText = 'Alarm CP';
				statusClass = 'status-alarm-cp';
				break;
			case 'ok':
			case 'available':
				statusText = 'OK';
				statusClass = 'status-ok';
				break;
			case 'charge':
			case 'charging':
				statusText = 'Charge';
				statusClass = 'status-charge';
				break;
			case 'in':
			case 'occupied':
				statusText = 'In';
				statusClass = 'status-in';
				break;
			case 'unavailable':
				statusText = 'Unavailable';
				statusClass = 'status-unavailable';
				break;
			default:
				statusText = 'Unknown';
				statusClass = 'status-unavailable';
		}

		connectorStatus.textContent = statusText;
		connectorStatus.className = `connector-status ${statusClass}`;
	}

	// OCPP Action Handlers
	async handleDisconnect() {
		if (!this.selectedStationId) {
			alert('Please select a station first');
			return;
		}

		this.addLog(`Отправка запроса на отключение станции: ${this.selectedStationId}`);

		if (this.connectionState === 'connected') {
			try {
				// Здесь можно добавить вызов RPC для отключения станции
				this.addLog(`Станция ${this.selectedStationId} отключена`);
			} catch (error) {
				this.addLog(`Ошибка отключения станции: ${error.message}`);
			}
		} else {
			// Демо режим
			this.stations[this.selectedStationId].status = 'disconnected';
			this.updateStationDisplay(this.selectedStationId, 'disconnected');
			this.addLog(`Станция ${this.selectedStationId} переведена в статус disconnected (демо)`);
		}
	}

	async handleGetBaseReport() {
		if (!this.selectedStationId) {
			alert('Please select a station first');
			return;
		}

		this.addLog(`Отправка GetBaseReport для станции: ${this.selectedStationId}`);

		if (this.connectionState === 'connected') {
			try {
				const response = await rpcClient.call('sendGetBaseReport', {
					chargePointId: this.selectedStationId
				});
				this.addLog(`GetBaseReport выполнен: ${JSON.stringify(response)}`);
			} catch (error) {
				this.addLog(`Ошибка GetBaseReport: ${error.message}`);
			}
		} else {
			// Демо режим
			this.addLog(`GetBaseReport выполнен для ${this.selectedStationId} (демо)`);
		}
	}

	async handleSendVariables() {
		const component = document.getElementById('component-input').value;
		const variable = document.getElementById('variable-input').value;
		const attribute = document.getElementById('attribute-input').value;

		if (!component || !variable) {
			alert('Please fill Component and Variable fields');
			return;
		}

		this.addLog(`Отправка SendVariables: Component=${component}, Variable=${variable}, Attribute=${attribute}`);

		if (this.connectionState === 'connected') {
			try {
				const response = await rpcClient.call('sendVariablesReq', {
					component: component,
					variable: variable,
					attributeType: attribute || 'Actual'
				});
				this.addLog(`SendVariables выполнен: ${JSON.stringify(response)}`);
			} catch (error) {
				this.addLog(`Ошибка SendVariables: ${error.message}`);
			}
		} else {
			// Демо режим
			this.addLog(`SendVariables выполнен (демо)`);
		}
	}

	async handleSendStatusNotification() {
		if (!this.selectedStationId || !this.selectedConnectorId) {
			alert('Please select station and connector first');
			return;
		}

		this.addLog(`Отправка TriggerStatusNotification для станции: ${this.selectedStationId}, коннектор: ${this.selectedConnectorId}`);

		if (this.connectionState === 'connected') {
			try {
				const response = await rpcClient.call('sendTriggerStatusNotification', {
					chargePointId: this.selectedStationId,
					connectorId: parseInt(this.selectedConnectorId)
				});
				this.addLog(`TriggerStatusNotification выполнен: ${JSON.stringify(response)}`);
			} catch (error) {
				this.addLog(`Ошибка TriggerStatusNotification: ${error.message}`);
			}
		} else {
			// Демо режим
			this.addLog(`TriggerStatusNotification выполнен (демо)`);
		}
	}

	async handleSendMeterValues() {
		const evseId = document.getElementById('evse-id-input').value || this.selectedConnectorId;

		if (!evseId) {
			alert('Please enter EVSE ID or select connector');
			return;
		}

		if (!this.selectedStationId) {
			alert('Please select station first');
			return;
		}

		this.addLog(`Отправка TriggerMeterValues для станции: ${this.selectedStationId}, EVSE: ${evseId}`);

		if (this.connectionState === 'connected') {
			try {
				const response = await rpcClient.call('sendTriggerMeterValues', {
					chargePointId: this.selectedStationId,
					evseId: parseInt(evseId)
				});
				this.addLog(`TriggerMeterValues выполнен: ${JSON.stringify(response)}`);
			} catch (error) {
				this.addLog(`Ошибка TriggerMeterValues: ${error.message}`);
			}
		} else {
			// Демо режим
			this.addLog(`TriggerMeterValues выполнен (демо)`);
		}
	}

	async handleSendHeartbit() {
		if (!this.selectedStationId) {
			alert('Please select station first');
			return;
		}

		this.addLog(`Отправка TriggerHeartbit для станции: ${this.selectedStationId}`);

		if (this.connectionState === 'connected') {
			try {
				const response = await rpcClient.call('sendTriggerHeartbit', {
					chargePointId: this.selectedStationId
				});
				this.addLog(`TriggerHeartbit выполнен: ${JSON.stringify(response)}`);
			} catch (error) {
				this.addLog(`Ошибка TriggerHeartbit: ${error.message}`);
			}
		} else {
			// Демо режим
			this.addLog(`TriggerHeartbit выполнен (демо)`);
		}
	}

	// Методы для обновления данных от сервера
	updateStationStatus(chargePointId, status) {
		if (this.stations[chargePointId]) {
			this.stations[chargePointId].status = status;
			if (this.selectedStationId === chargePointId) {
				this.updateStationDisplay(chargePointId, status);
			}
		}
	}

	updateConnectorStatus(chargePointId, connectorId, status) {
		if (this.stations[chargePointId] && this.stations[chargePointId].connectors[connectorId]) {
			this.stations[chargePointId].connectors[connectorId].status = status;
			if (this.selectedStationId === chargePointId && this.selectedConnectorId === connectorId.toString()) {
				this.updateConnectorStatusDisplay(status);
			}
		}
	}

	updateConnectorMeterValues(chargePointId, connectorId, meterValue) {
		if (this.stations[chargePointId] && this.stations[chargePointId].connectors[connectorId]) {
			// Обработка значений счетчика
			if (meterValue && meterValue.length > 0) {
				const sampledValue = meterValue[0].sampledValue;
				if (sampledValue && sampledValue.length > 0) {
					const value = sampledValue[0].value;
					const context = sampledValue[0].context;

					if (context === 'Transaction.Begin' || context === 'Transaction.End') {
						this.stations[chargePointId].connectors[connectorId].meter = value;
					} else if (context === 'Sample.Periodic') {
						this.stations[chargePointId].connectors[connectorId].power = value;
					}

					if (this.selectedStationId === chargePointId && this.selectedConnectorId === connectorId.toString()) {
						document.getElementById('connector-meter').textContent = this.stations[chargePointId].connectors[connectorId].meter;
						document.getElementById('connector-power').textContent = this.stations[chargePointId].connectors[connectorId].power;
					}
				}
			}
		}
	}

	// Логирование
	addLog(message) {
		const logContent = document.getElementById('log-content');
		const timestamp = new Date().toLocaleTimeString();
		const logEntry = document.createElement('div');
		logEntry.className = 'log-entry';
		logEntry.innerHTML = `<span class="log-timestamp">[${timestamp}]</span> ${message}`;
		logContent.appendChild(logEntry);
		logContent.scrollTop = logContent.scrollHeight;
	}

	clearLog() {
		const logContent = document.getElementById('log-content');
		logContent.innerHTML = '';
		this.addLog("Лог очищен");
	}
}

// Инициализация системы при загрузке страницы
document.addEventListener('DOMContentLoaded', () => {
	window.ocppSystem = new OCPPSystem();
});