class MonitoringSystem {
	constructor() {
		this.values = {};
		this.connectionState = 'disconnected';
		this.init();
	}

	init() {
		console.log('Initializing MonitoringSystem...');

		rpcClient.on('connected', () => {
			console.log('RPC client connected successfully');
			this.updateConnectionStatus(true);
			this.connectionState = 'connected';
			this.startMonitoring();

			// Временная отладка
			setTimeout(() => this.debugMetrics(), 1000);
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

		rpcClient.on('valueChanged', (params) => {
			console.log('Value changed notification:', params);
			this.updateValue(params.key, params.value);
		});

		rpcClient.on('chargingStarted', (params) => {
			this.addLog(`Charging started - Session: ${params.sessionId}`);
			this.updateValue('systemStatus', 'charging');
			this.updateValue('chargingEnabled', 'true');
		});

		rpcClient.on('chargingStopped', (params) => {
			this.addLog(`Charging stopped - Energy: ${parseFloat(params.energyConsumed)} kWh`);
			this.updateValue('systemStatus', 'ready');
			this.updateValue('chargingEnabled', 'false');
		});

		rpcClient.on('metricsUpdate', (params) => {
			if (params.currentPower !== undefined) {
				this.updateValue('currentPower', parseFloat(params.currentPower));
			}
			if (params.voltage !== undefined) {
				this.updateValue('voltage', parseFloat(params.voltage));
			}
			if (params.temperature !== undefined) {
				this.updateValue('temperature', parseFloat(params.temperature));
			}
		});

		// Подключаемся к серверу
		rpcClient.connect();

		// Настраиваем обработчики кнопок
		this.setupEventHandlers();
	}

	async testRpcConnection() {
		try {
			this.addLog('Testing RPC connection...');
			const result = await rpcClient.call('getValue', { key: 'systemStatus' });
			this.addLog('RPC test successful: ' + result);
			console.log('RPC test result:', result);
		} catch (error) {
			this.addLog('RPC test failed: ' + error.message);
			console.error('RPC test error:', error);
		}
	}

	async startMonitoring() {
		try {
			// Запрашиваем начальные значения
			const valuesResponse = await rpcClient.call('getValues');

			console.log('Raw values response:', valuesResponse);

			// Обрабатываем двойной JSON
			let values;
			if (typeof valuesResponse === 'string') {
				try {
					const parsed = JSON.parse(valuesResponse);
					values = parsed.result ? JSON.parse(parsed.result) : parsed;
				} catch (e) {
					values = valuesResponse;
				}
			} else if (valuesResponse && typeof valuesResponse.result === 'string') {
				values = JSON.parse(valuesResponse.result);
			} else {
				values = valuesResponse;
			}

			console.log('Parsed values:', values);

			// Обновляем интерфейс
			if (values && typeof values === 'object') {
				Object.keys(values).forEach(key => {
					this.updateValue(key, values[key]);
				});
			}

			// Запускаем периодическое обновление метрик
			setInterval(() => this.updateMetrics(), 1000);

			this.addLog('Monitoring started successfully');

		} catch (error) {
			console.error('Failed to start monitoring:', error);
			this.addLog('Error starting monitoring: ' + error.message);
		}
	}

	async updateMetrics() {
		try {
			const response = await rpcClient.call('getMetrics');

			// Простое решение - извлекаем result и парсим его
			let metricsData;
			if (response && response.result) {
				metricsData = JSON.parse(response.result);
			} else {
				metricsData = response;
			}

			console.log('Metrics data:', metricsData);

			// Обновляем значения
			if (metricsData && typeof metricsData === 'object') {
				this.updateValue('currentPower', parseFloat(metricsData.currentPower));
				this.updateValue('voltage', parseFloat(metricsData.voltage));
				this.updateValue('temperature', parseFloat(metricsData.temperature));
				this.updateValue('connectedClients', metricsData.connectedClients);
				this.updateValue('uptime', metricsData.uptime);
			}

			document.getElementById('serverTime').textContent =
				new Date().toLocaleTimeString();

		} catch (error) {
			console.error('Error updating metrics:', error);
			this.addLog('Error updating metrics: ' + error.message);
		}
	}

	updateValue(key, value) {
		try {
			// Проверяем что value не undefined/null
			if (value === undefined || value === null) {
				console.warn(`Value for ${key} is undefined or null`);
				value = 'N/A';
			}

			this.values[key] = value;

			const element = document.getElementById(key);
			if (!element) {
				console.warn(`Element with id ${key} not found`);
				return;
			}

			// Преобразуем value в строку для безопасной обработки
			const stringValue = String(value);

			// Специальная обработка для разных типов значений
			switch (key) {
				case 'currentPower':
					const powerValue = parseFloat(stringValue);
					if (!isNaN(powerValue)) {
						element.textContent = parseFloat(powerValue) + ' kW';
						this.updatePowerBar(powerValue);
					} else {
						element.textContent = '0.00 kW';
						this.updatePowerBar(0);
					}
					break;
				case 'voltage':
					const voltageValue = parseFloat(stringValue);
					if (!isNaN(voltageValue)) {
						element.textContent = parseFloat(voltageValue) + ' V';
					} else {
						element.textContent = '0.0 V';
					}
					break;
				case 'temperature':
					const tempValue = parseFloat(stringValue);
					if (!isNaN(tempValue)) {
						element.textContent = parseFloat(tempValue) + '°C';
						this.updateTemperatureColor(tempValue, element);
					} else {
						element.textContent = '0.0°C';
						this.updateTemperatureColor(0, element);
					}
					break;
				case 'uptime':
					const uptimeValue = parseInt(stringValue);
					if (!isNaN(uptimeValue)) {
						element.textContent = this.formatUptime(uptimeValue);
					} else {
						element.textContent = '0s';
					}
					break;
				case 'systemStatus':
					element.textContent = stringValue;
					this.updateStatusColor(stringValue, element);
					break;
				case 'chargingEnabled':
					// Обновляем состояние кнопок
					this.updateButtonStates(stringValue === 'true' || stringValue === '1');
					break;
				default:
					element.textContent = stringValue;
			}
		} catch (error) {
			console.error(`Error in updateValue for key ${key}:`, error);
		}
	}

	async debugMetrics() {
		try {
			const response = await rpcClient.call('getMetrics');
			console.log('DEBUG getMetrics response:', response);
			console.log('Type of response:', typeof response);

			if (typeof response === 'string') {
				console.log('Attempting to parse as JSON...');
				const parsed = JSON.parse(response);
				console.log('Parsed metrics:', parsed);
			}
		} catch (error) {
			console.error('Debug metrics error:', error);
		}
	}

	updateButtonStates(chargingEnabled) {
		const startBtn = document.getElementById('startCharging');
		const stopBtn = document.getElementById('stopCharging');

		if (chargingEnabled) {
			startBtn.disabled = true;
			stopBtn.disabled = false;
			startBtn.style.opacity = '0.6';
			stopBtn.style.opacity = '1';
		} else {
			startBtn.disabled = false;
			stopBtn.disabled = true;
			startBtn.style.opacity = '1';
			stopBtn.style.opacity = '0.6';
		}
	}

	updateStatusColor(status, element) {
		const colors = {
			'ready': '#27ae60',
			'charging': '#3498db',
			'error': '#e74c3c',
			'maintenance': '#f39c12'
		};
		element.style.color = colors[status] || '#2c3e50';
	}

	updatePowerBar(power) {
		const maxPower = parseInt(this.values['maxPower'] || 50);
		const percentage = Math.min((power / maxPower) * 100, 100);
		const powerBar = document.getElementById('powerBar');
		if (powerBar) {
			powerBar.style.width = percentage + '%';

			// Меняем цвет в зависимости от нагрузки
			if (percentage > 90) {
				powerBar.style.background = 'linear-gradient(90deg, #e74c3c, #c0392b)';
			} else if (percentage > 70) {
				powerBar.style.background = 'linear-gradient(90deg, #f39c12, #e67e22)';
			} else {
				powerBar.style.background = 'linear-gradient(90deg, #27ae60, #2ecc71)';
			}
		}
	}

	updateTemperatureColor(temp, element) {
		if (temp > 60) {
			element.style.color = '#e74c3c';
		} else if (temp > 45) {
			element.style.color = '#f39c12';
		} else {
			element.style.color = '#27ae60';
		}
	}

	formatUptime(seconds) {
		const days = Math.floor(seconds / 86400);
		const hours = Math.floor((seconds % 86400) / 3600);
		const minutes = Math.floor((seconds % 3600) / 60);
		const secs = seconds % 60;

		if (days > 0) {
			return `${days}d ${hours}h ${minutes}m`;
		} else if (hours > 0) {
			return `${hours}h ${minutes}m ${secs}s`;
		} else {
			return `${minutes}m ${secs}s`;
		}
	}

	updateConnectionStatus(connected) {
		const statusElement = document.getElementById('connectionStatus');
		if (connected) {
			statusElement.textContent = 'Online';
			statusElement.className = 'status-online';
		} else {
			statusElement.textContent = 'Offline';
			statusElement.className = 'status-offline';
		}
	}

	updateSystemStatus(status) {
		const element = document.getElementById('systemStatus');
		if (element) {
			element.textContent = status.status;
			element.style.color = status.status === 'running' ? '#27ae60' :
				status.status === 'warning' ? '#f39c12' : '#e74c3c';
		}
	}

	setupEventHandlers() {
		console.log('Setting up event handlers...');

		// Обработчики кнопок управления
		document.getElementById('startCharging').addEventListener('click', () => {
			console.log('Start Charging button clicked');
			this.sendCommand('startCharging', {});
		});


		document.getElementById('stopCharging').addEventListener('click', () => {
			console.log('Stop Charging button clicked');
			this.sendCommand('stopCharging', {});
		});


		document.getElementById('resetSystem').addEventListener('click', () => {
			console.log('Reset System button clicked');
			this.sendCommand('resetSystem', {});
		});

		document.getElementById('setMaxPower').addEventListener('click', () => {
			const maxPower = document.getElementById('maxPower').value;
			console.log('Set Max Power button clicked:', maxPower);
			if (maxPower && maxPower >= 1 && maxPower <= 100) {
				this.sendCommand('setMaxPower', { value: parseInt(maxPower) });
			} else {
				alert('Please enter a valid power value between 1 and 100 kW');
			}
		});

		// Добавим тестовую кнопку для проверки
		this.addTestButton();
	}

	async sendCommand(command, params = {}) {
		console.log(`=== Sending Command: ${command} ===`, params);
		this.addLog(`Sending command: ${command}`);

		if (this.connectionState !== 'connected') {
			const error = 'Not connected to server. Please check connection.';
			console.error('Not connected to server');
			this.addLog('Error: ' + error);
			alert(error);
			return;
		}

		// Проверяем состояние WebSocket
		if (!rpcClient.ws) {
			console.error('WebSocket instance not found');
			alert('WebSocket not initialized');
			return;
		}

		if (rpcClient.ws.readyState !== WebSocket.OPEN) {
			console.error('WebSocket not open. State:', rpcClient.ws.readyState);
			alert('WebSocket connection not open');
			return;
		}

		try {
			console.log('Calling RPC method...');
			const result = await rpcClient.call(command, params);
			console.log('Command successful:', result);
			this.addLog(`Command ${command} executed: ${result}`);
			return result;
		} catch (error) {
			console.error('Command failed:', error);
			const errorMsg = `Error executing ${command}: ${error.message}`;
			this.addLog(errorMsg);
			alert(errorMsg);
		}
	}

	// Добавьте метод для тестирования соединения
	testConnection() {
		console.log('=== Testing Connection ===');
		console.log('RPC Client:', rpcClient);
		console.log('WebSocket:', rpcClient.ws);
		if (rpcClient.ws) {
			console.log('WebSocket state:', rpcClient.ws.readyState);
			console.log('WebSocket URL:', rpcClient.ws.url);
		}
		console.log('Connected:', rpcClient.connected);
		console.log('Connection state:', this.connectionState);
	}

	// Добавьте кнопку для тестирования в setupEventHandlers
	addTestButton() {
		const testBtn = document.createElement('button');
		testBtn.textContent = 'Debug Test';
		testBtn.className = 'btn btn-secondary';
		testBtn.onclick = () => this.debugTest();
		document.querySelector('.controls').appendChild(testBtn);
	}

	debugTest() {
		console.log('=== Debug Test ===');

		if (!window.rpcClient || !window.rpcClient.ws) {
			console.error('RPC client not initialized');
			return;
		}

		console.log('WebSocket state:', rpcClient.ws.readyState);
		console.log('WebSocket URL:', rpcClient.ws.url);
		console.log('Connected:', rpcClient.connected);

		// Простой тест - отправляем сырое сообщение
		const testMsg = {
			jsonrpc: "2.0",
			id: 1,
			method: "getValue",
			params: { key: "systemStatus" }
		};

		console.log('Sending test message:', testMsg);

		try {
			rpcClient.ws.send(JSON.stringify(testMsg));
			console.log('Test message sent successfully');
		} catch (error) {
			console.error('Error sending test message:', error);
		}
	}


	addLog(message) {
		const logContainer = document.getElementById('eventLog');
		const logEntry = document.createElement('div');
		logEntry.className = 'log-entry';
		logEntry.textContent = `[${new Date().toLocaleTimeString()}] ${message}`;

		logContainer.appendChild(logEntry);
		logContainer.scrollTop = logContainer.scrollHeight;

		// Ограничиваем количество записей
		if (logContainer.children.length > 50) {
			logContainer.removeChild(logContainer.firstChild);
		}
	}

	async updateMetrics() {
		try {
			console.log('=== Starting updateMetrics ===');
			const response = await rpcClient.call('getMetrics');

			console.log('Raw RPC response:', response);
			console.log('Response type:', typeof response);

			// Детальная отладка структуры ответа
			if (response) {
				console.log('Response keys:', Object.keys(response));
				if (response.result) {
					console.log('Result type:', typeof response.result);
					console.log('Result value:', response.result);
				}
			}

			let metricsData;

			// Обрабатываем разные возможные форматы ответа
			if (response && response.result) {
				if (typeof response.result === 'string') {
					try {
						metricsData = JSON.parse(response.result);
						console.log('Parsed result string:', metricsData);
					} catch (parseError) {
						console.error('Failed to parse result string:', parseError);
						// Если не получается распарсить, используем как есть
						metricsData = response.result;
					}
				} else {
					// Если result уже объект
					metricsData = response.result;
				}
			} else if (response && typeof response === 'object') {
				// Если response уже содержит данные метрик
				metricsData = response;
			} else {
				console.error('Unexpected response format:', response);
				this.addLog('Error: Unexpected metrics format');
				return;
			}

			console.log('Final metrics data:', metricsData);
			console.log('Metrics data type:', typeof metricsData);

			// БЕЗОПАСНОЕ обновление значений
			if (metricsData && typeof metricsData === 'object') {
				// currentPower
				if (metricsData.currentPower !== undefined && metricsData.currentPower !== null) {
					const power = parseFloat(metricsData.currentPower);
					if (!isNaN(power)) {
						this.updateValue('currentPower', parseFloat(power));
					} else {
						console.warn('Invalid currentPower value:', metricsData.currentPower);
						this.updateValue('currentPower', '0.00');
					}
				} else {
					console.warn('currentPower is undefined or null');
					this.updateValue('currentPower', '0.00');
				}

				// voltage
				if (metricsData.voltage !== undefined && metricsData.voltage !== null) {
					const voltage = parseFloat(metricsData.voltage);
					if (!isNaN(voltage)) {
						this.updateValue('voltage', parseFloat(voltage));
					} else {
						console.warn('Invalid voltage value:', metricsData.voltage);
						this.updateValue('voltage', '0.0');
					}
				} else {
					console.warn('voltage is undefined or null');
					this.updateValue('voltage', '0.0');
				}

				// temperature
				if (metricsData.temperature !== undefined && metricsData.temperature !== null) {
					const temp = parseFloat(metricsData.temperature);
					if (!isNaN(temp)) {
						this.updateValue('temperature', parseFloat(temp));
					} else {
						console.warn('Invalid temperature value:', metricsData.temperature);
						this.updateValue('temperature', '0.0');
					}
				} else {
					console.warn('temperature is undefined or null');
					this.updateValue('temperature', '0.0');
				}

				// connectedClients
				if (metricsData.connectedClients !== undefined && metricsData.connectedClients !== null) {
					this.updateValue('connectedClients', metricsData.connectedClients);
				} else {
					console.warn('connectedClients is undefined or null');
					this.updateValue('connectedClients', '0');
				}

				// uptime
				if (metricsData.uptime !== undefined && metricsData.uptime !== null) {
					this.updateValue('uptime', metricsData.uptime);
				} else {
					console.warn('uptime is undefined or null');
					this.updateValue('uptime', '0');
				}

			} else {
				console.error('Metrics data is not an object:', metricsData);
				this.addLog('Error: Metrics data format invalid');

				// Устанавливаем значения по умолчанию при ошибке
				this.updateValue('currentPower', '0.00');
				this.updateValue('voltage', '0.0');
				this.updateValue('temperature', '0.0');
				this.updateValue('connectedClients', '0');
				this.updateValue('uptime', '0');
			}

			// Обновляем время сервера
			const serverTimeElement = document.getElementById('serverTime');
			if (serverTimeElement) {
				serverTimeElement.textContent = new Date().toLocaleTimeString();
			}

			console.log('=== updateMetrics completed ===');

		} catch (error) {
			console.error('Error in updateMetrics:', error);
			this.addLog('Error updating metrics: ' + error.message);

			// Устанавливаем значения по умолчанию при ошибке
			this.updateValue('currentPower', '0.00');
			this.updateValue('voltage', '0.0');
			this.updateValue('temperature', '0.0');
			this.updateValue('connectedClients', '0');
			this.updateValue('uptime', '0');
		}
	}

	async testMetricsDirectly() {
		try {
			console.log('=== Testing getMetrics directly ===');
			const testResponse = await rpcClient.call('getMetrics');
			console.log('Direct test response:', testResponse);

			// Попробуем разные способы доступа к данным
			console.log('Different access attempts:');
			console.log('- testResponse.result:', testResponse?.result);
			console.log('- testResponse.currentPower:', testResponse?.currentPower);
			console.log('- testResponse.voltage:', testResponse?.voltage);

			if (testResponse?.result) {
				console.log('- typeof result:', typeof testResponse.result);
				if (typeof testResponse.result === 'string') {
					try {
						const parsed = JSON.parse(testResponse.result);
						console.log('- Parsed result:', parsed);
					} catch (e) {
						console.log('- Cannot parse result as JSON');
					}
				}
			}
		} catch (error) {
			console.error('Direct test failed:', error);
		}
	}
}

// Запускаем систему мониторинга при загрузке страницы
document.addEventListener('DOMContentLoaded', () => {
	window.monitoringSystem = new MonitoringSystem();
});

window.testRpc = function () {
	console.log('=== Manual RPC Test ===');

	if (!window.rpcClient || !window.rpcClient.connected) {
		console.error('RPC client not connected');
		alert('Please connect first');
		return;
	}

	const testMessage = {
		jsonrpc: "2.0",
		id: 999,
		method: "getValue",
		params: { "key": "systemStatus" }
	};

	console.log('Sending test message:', testMessage);

	// Отправляем напрямую через WebSocket
	window.rpcClient.ws.send(JSON.stringify(testMessage));
	console.log('Test message sent');
};
