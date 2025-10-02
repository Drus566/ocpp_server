// station-manager.js

/**
 * Менеджер управления станцией
 * Следует принципам SOLID и лучшим практикам JavaScript
 */
class StationManager {
    constructor(rpcClient) {
        this.rpcClient = rpcClient;
        this.currentStation = null;
        this.currentConnector = null;
        this.stations = new Map();
        this.isInitialized = false;

        this.init();
    }

    /**
     * Инициализация менеджера
     */
    init() {
        if (this.isInitialized) return;

        this.bindEvents();
        this.setupRpcHandlers();
        this.loadInitialData();
        
        this.isInitialized = true;
        console.log('StationManager initialized');
    }

    /**
     * Привязка DOM событий
     */
    bindEvents() {
        // Выбор станции
        document.getElementById('station-select').addEventListener('change', (e) => {
            this.handleStationChange(e.target.value);
        });

        // Выбор коннектора
        document.getElementById('connector-select').addEventListener('change', (e) => {
            this.handleConnectorChange(e.target.value);
        });

        // Кнопки управления станцией
        this.bindStationControls();
        
        // Кнопки управления коннектором
        this.bindConnectorControls();

        // Trigger кнопки
        this.bindTriggerControls();
    }

    /**
     * Настройка обработчиков RPC событий
     */
    setupRpcHandlers() {
        this.rpcClient.on('connected', () => {
            this.updateConnectionStatus(true);
            this.loadStationsList();
        });

        this.rpcClient.on('disconnected', () => {
            this.updateConnectionStatus(false);
        });

        this.rpcClient.on('StatusNotification', (data) => {
            this.handleStatusNotification(data);
        });

        this.rpcClient.on('MeterValues', (data) => {
            this.handleMeterValues(data);
        });

        this.rpcClient.on('Heartbeat', (data) => {
            this.handleHeartbeat(data);
        });
    }

    /**
     * Загрузка начальных данных
     */
    async loadInitialData() {
        try {
            // Загрузка списка станций при старте
            if (this.rpcClient.connected) {
                await this.loadStationsList();
            }
        } catch (error) {
            console.error('Error loading initial data:', error);
            this.logError('Ошибка загрузки начальных данных');
        }
    }

    /**
     * Загрузка списка станций
     */
    async loadStationsList() {
        try {
            this.log('Загрузка списка станций...');
            
            const response = await this.rpcClient.call('GetStations');
            const stations = this.parseResponse(response);
            
            this.updateStationsDropdown(stations);
            this.cacheStationsData(stations);
            
            // Автоматически выбираем первую станцию
            if (stations.length > 0 && !this.currentStation) {
                this.handleStationChange(stations[0].id);
            }
            
        } catch (error) {
            console.error('Error loading stations list:', error);
            this.logError('Ошибка загрузки списка станций');
        }
    }

    /**
     * Обработчик изменения выбранной станции
     */
    async handleStationChange(stationId) {
        try {
            this.log(`Выбор станции: ${stationId}`);
            
            const response = await this.rpcClient.call('GetStationStatus', { stationId });
            const stationData = this.parseResponse(response);
            
            this.currentStation = stationId;
            this.updateStationInfo(stationData);
            this.updateConnectorsDropdown(stationData.connectors);
            
            // Выбираем первый коннектор по умолчанию
            if (stationData.connectors && stationData.connectors.length > 0) {
                this.handleConnectorChange(stationData.connectors[0].id);
            }
            
        } catch (error) {
            console.error('Error changing station:', error);
            this.logError(`Ошибка загрузки данных станции ${stationId}`);
        }
    }

    /**
     * Обработчик изменения выбранного коннектора
     */
    async handleConnectorChange(connectorId) {
        try {
            this.log(`Выбор коннектора: ${connectorId}`);
            
            const response = await this.rpcClient.call('GetConnectorStatus', {
                stationId: this.currentStation,
                connectorId
            });
            
            const connectorData = this.parseResponse(response);
            this.currentConnector = connectorId;
            this.updateConnectorInfo(connectorData);
            
        } catch (error) {
            console.error('Error changing connector:', error);
            this.logError(`Ошибка загрузки данных коннектора ${connectorId}`);
        }
    }

    /**
     * Привязка обработчиков для кнопок управления станцией
     */
    bindStationControls() {
        // Отключить станцию
        document.querySelector('.btn--danger').addEventListener('click', () => {
            this.disableStation();
        });

        // Получить информацию
        document.querySelector('.btn-with-tooltip .btn--primary').addEventListener('click', () => {
            this.getStationInfo();
        });

        // Получить конфигурацию
        document.querySelectorAll('.btn--primary')[2].addEventListener('click', () => {
            this.getConfiguration();
        });

        // Получить переменную
        document.querySelectorAll('.btn-with-tooltip .btn--primary')[1].addEventListener('click', () => {
            this.getVariable();
        });

        // Записать переменную
        document.querySelector('.variable-control .btn--primary').addEventListener('click', () => {
            this.setVariable();
        });

        // Перезагрузка
        document.querySelectorAll('.btn-with-tooltip .btn--warning')[0].addEventListener('click', () => {
            this.rebootStation();
        });

        // Очистить кэш
        document.querySelectorAll('.btn-with-tooltip .btn--warning')[1].addEventListener('click', () => {
            this.clearCache();
        });
    }

    /**
     * Привязка обработчиков для кнопок управления коннектором
     */
    bindConnectorControls() {
        const connectorButtons = document.querySelectorAll('.connector-controls .btn--primary');
        
        connectorButtons[0].addEventListener('click', () => {
            this.requestStartTransaction();
        });

        connectorButtons[1].addEventListener('click', () => {
            this.requestStopTransaction();
        });

        connectorButtons[2].addEventListener('click', () => {
            this.unlockConnector();
        });
    }

    /**
     * Привязка обработчиков для Trigger кнопок
     */
    bindTriggerControls() {
        const triggerButtons = document.querySelectorAll('.trigger-controls .btn--secondary');
        
        triggerButtons[0].addEventListener('click', () => {
            this.triggerMeterValues();
        });

        triggerButtons[1].addEventListener('click', () => {
            this.triggerStatusNotification();
        });

        triggerButtons[2].addEventListener('click', () => {
            this.triggerHeartbeat();
        });
    }

    // === RPC МЕТОДЫ ДЛЯ СТАНЦИИ ===

    async disableStation() {
        if (!this.validateCurrentStation()) return;
        
        try {
            this.log(`Отключение станции ${this.currentStation}`);
            await this.rpcClient.call('DisableStation', {
                stationId: this.currentStation
            });
            this.logSuccess('Станция успешно отключена');
        } catch (error) {
            this.handleRpcError('Ошибка отключения станции', error);
        }
    }

    async getStationInfo() {
        if (!this.validateCurrentStation()) return;
        
        try {
            const response = await this.rpcClient.call('GetStationInfo', {
                stationId: this.currentStation
            });
            this.logSuccess('Информация о станции получена');
            this.log(`Информация о станции: ${JSON.stringify(this.parseResponse(response))}`);
        } catch (error) {
            this.handleRpcError('Ошибка получения информации о станции', error);
        }
    }

    async getConfiguration() {
        if (!this.validateCurrentStation()) return;
        
        try {
            const response = await this.rpcClient.call('GetConfiguration', {
                stationId: this.currentStation
            });
            this.logSuccess('Конфигурация получена');
            this.log(`Конфигурация: ${JSON.stringify(this.parseResponse(response))}`);
        } catch (error) {
            this.handleRpcError('Ошибка получения конфигурации', error);
        }
    }

    async getVariable() {
        if (!this.validateCurrentStation()) return;
        
        try {
            const response = await this.rpcClient.call('GetVariable', {
                stationId: this.currentStation
            });
            this.logSuccess('Переменные получены');
            this.log(`Переменные: ${JSON.stringify(this.parseResponse(response))}`);
        } catch (error) {
            this.handleRpcError('Ошибка получения переменных', error);
        }
    }

    async setVariable() {
        if (!this.validateCurrentStation()) return;
        
        const inputs = document.querySelectorAll('.variable-control__inputs .input-field');
        const [component, variable, attribute] = Array.from(inputs).map(input => input.value);
        
        if (!component || !variable) {
            this.logError('Заполните обязательные поля: Component и Variable');
            return;
        }
        
        try {
            await this.rpcClient.call('SetVariable', {
                stationId: this.currentStation,
                component,
                variable,
                attribute: attribute || undefined
            });
            this.logSuccess('Переменная успешно записана');
        } catch (error) {
            this.handleRpcError('Ошибка записи переменной', error);
        }
    }

    async rebootStation() {
        if (!this.validateCurrentStation()) return;
        
        try {
            await this.rpcClient.call('RebootStation', {
                stationId: this.currentStation
            });
            this.logSuccess('Команда перезагрузки отправлена');
        } catch (error) {
            this.handleRpcError('Ошибка перезагрузки станции', error);
        }
    }

    async clearCache() {
        if (!this.validateCurrentStation()) return;
        
        try {
            await this.rpcClient.call('ClearCache', {
                stationId: this.currentStation
            });
            this.logSuccess('Кэш успешно очищен');
        } catch (error) {
            this.handleRpcError('Ошибка очистки кэша', error);
        }
    }

    // === RPC МЕТОДЫ ДЛЯ КОННЕКТОРА ===

    async requestStartTransaction() {
        if (!this.validateCurrentConnector()) return;
        
        try {
            await this.rpcClient.call('RequestStartTransaction', {
                stationId: this.currentStation,
                connectorId: this.currentConnector
            });
            this.logSuccess('Запрос на начало транзакции отправлен');
        } catch (error) {
            this.handleRpcError('Ошибка начала транзакции', error);
        }
    }

    async requestStopTransaction() {
        if (!this.validateCurrentConnector()) return;
        
        try {
            await this.rpcClient.call('RequestStopTransaction', {
                stationId: this.currentStation,
                connectorId: this.currentConnector
            });
            this.logSuccess('Запрос на остановку транзакции отправлен');
        } catch (error) {
            this.handleRpcError('Ошибка остановки транзакции', error);
        }
    }

    async unlockConnector() {
        if (!this.validateCurrentConnector()) return;
        
        try {
            await this.rpcClient.call('UnlockConnector', {
                stationId: this.currentStation,
                connectorId: this.currentConnector
            });
            this.logSuccess('Команда разблокировки коннектора отправлена');
        } catch (error) {
            this.handleRpcError('Ошибка разблокировки коннектора', error);
        }
    }

    // === TRIGGER МЕТОДЫ ===

    async triggerMeterValues() {
        if (!this.validateCurrentStation()) return;
        
        const evseIdInput = document.querySelector('.trigger-item .input-field');
        const evseId = evseIdInput.value || this.currentConnector;
        
        try {
            await this.rpcClient.call('TriggerMeterValues', {
                stationId: this.currentStation,
                evseId
            });
            this.logSuccess(`Meter values triggered for EVSE: ${evseId}`);
        } catch (error) {
            this.handleRpcError('Ошибка trigger MeterValues', error);
        }
    }

    async triggerStatusNotification() {
        if (!this.validateCurrentStation()) return;
        
        try {
            await this.rpcClient.call('TriggerStatusNotification', {
                stationId: this.currentStation
            });
            this.logSuccess('Status notification triggered');
        } catch (error) {
            this.handleRpcError('Ошибка trigger StatusNotification', error);
        }
    }

    async triggerHeartbeat() {
        if (!this.validateCurrentStation()) return;
        
        try {
            await this.rpcClient.call('TriggerHeartbeat', {
                stationId: this.currentStation
            });
            this.logSuccess('Heartbeat triggered');
        } catch (error) {
            this.handleRpcError('Ошибка trigger Heartbeat', error);
        }
    }

    // === ОБРАБОТЧИКИ RPC УВЕДОМЛЕНИЙ ===

    handleStatusNotification(data) {
        this.log(`StatusNotification: ${JSON.stringify(data)}`);
        
        if (data.stationId === this.currentStation) {
            this.updateStationStatus(data.status);
            
            if (data.connectorId === this.currentConnector) {
                this.updateConnectorStatus(data.connectorStatus);
            }
        }
    }

    handleMeterValues(data) {
        this.log(`MeterValues: ${JSON.stringify(data)}`);
        
        if (data.stationId === this.currentStation && data.connectorId === this.currentConnector) {
            this.updateMeterValues(data.values);
        }
    }

    handleHeartbeat(data) {
        this.log(`Heartbeat received from station: ${data.stationId}`);
    }

    // === ОБНОВЛЕНИЕ UI ===

    updateStationsDropdown(stations) {
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

    updateStationInfo(stationData) {
        // Обновление статуса
        const statusElement = document.querySelector('.station-info .status--working');
        if (statusElement) {
            statusElement.textContent = this.getStatusText(stationData.status);
            statusElement.className = `info-item__value status--${stationData.status}`;
        }
        
        // Обновление количества коннекторов
        const connectorsCountElement = document.querySelector('.station-info__grid .info-item:nth-child(2) .info-item__value');
        if (connectorsCountElement && stationData.connectors) {
            connectorsCountElement.textContent = stationData.connectors.length;
        }
        
        // Обновление максимальной мощности
        const powerElement = document.querySelector('.station-info__grid .info-item:nth-child(3) .info-item__value');
        if (powerElement && stationData.maxPower) {
            powerElement.textContent = `${stationData.maxPower} кВт`;
        }
    }

    updateConnectorInfo(connectorData) {
        const grid = document.querySelector('.connector-info__grid');
        const infoItems = grid.querySelectorAll('.info-item');
        
        // Тип коннектора
        infoItems[0].querySelector('.info-item__value').textContent = 
            this.getConnectorTypeText(connectorData.type);
        
        // Статус
        const statusElement = infoItems[1].querySelector('.info-item__value');
        statusElement.textContent = this.getConnectorStatusText(connectorData.status);
        statusElement.className = `info-item__value status-indicator status-indicator--${connectorData.status}`;
        
        // Meter
        if (connectorData.meter !== undefined) {
            infoItems[2].querySelector('.info-item__value').textContent = 
                `${connectorData.meter} kWh`;
        }
        
        // Power
        if (connectorData.power !== undefined) {
            infoItems[3].querySelector('.info-item__value').textContent = 
                `${connectorData.power} kW`;
        }
    }

    updateStationStatus(status) {
        const statusElement = document.querySelector('.station-info .info-item__value');
        if (statusElement) {
            statusElement.textContent = this.getStatusText(status);
            statusElement.className = `info-item__value status--${status}`;
        }
    }

    updateConnectorStatus(status) {
        const statusElement = document.querySelector('.connector-info .status-indicator');
        if (statusElement) {
            statusElement.textContent = this.getConnectorStatusText(status);
            statusElement.className = `info-item__value status-indicator status-indicator--${status}`;
        }
    }

    updateMeterValues(values) {
        // Обновление значений счетчика и мощности
        if (values.meter !== undefined) {
            const meterElement = document.querySelector('.connector-info__grid .info-item:nth-child(3) .info-item__value');
            if (meterElement) {
                meterElement.textContent = `${values.meter} kWh`;
            }
        }
        
        if (values.power !== undefined) {
            const powerElement = document.querySelector('.connector-info__grid .info-item:nth-child(4) .info-item__value');
            if (powerElement) {
                powerElement.textContent = `${values.power} kW`;
            }
        }
    }

    updateConnectionStatus(connected) {
        const statusIndicator = document.createElement('div');
        statusIndicator.className = `connection-status ${connected ? 'connected' : 'disconnected'}`;
        statusIndicator.textContent = connected ? 'Подключено' : 'Отключено';
        
        let existingStatus = document.querySelector('.connection-status');
        if (existingStatus) {
            existingStatus.replaceWith(statusIndicator);
        } else {
            document.querySelector('.header').appendChild(statusIndicator);
        }
    }

    // === ВСПОМОГАТЕЛЬНЫЕ МЕТОДЫ ===

    validateCurrentStation() {
        if (!this.currentStation) {
            this.logError('Сначала выберите станцию');
            return false;
        }
        return true;
    }

    validateCurrentConnector() {
        if (!this.currentConnector) {
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
            'disabled': 'Отключена'
        };
        return statusMap[status] || status;
    }

    getConnectorTypeText(type) {
        const typeMap = {
            'GBT': 'GBT',
            'CCS2': 'CCS2',
            'TYPE2': 'TYPE2',
            'CHADEMO': 'CHADEMO'
        };
        return typeMap[type] || type;
    }

    getConnectorStatusText(status) {
        const statusMap = {
            'alarm-ev': 'Alarm EV',
            'alarm-cp': 'Alarm CP',
            'ok': 'OK',
            'charge': 'Charge',
            'in': 'In',
            'unavailable': 'Unavailable'
        };
        return statusMap[status] || status;
    }

    // === ЛОГИРОВАНИЕ ===

    log(message) {
        this.addLogEntry(message, 'info');
        console.log(`[StationManager] ${message}`);
    }

    logSuccess(message) {
        this.addLogEntry(message, 'success');
        console.log(`[StationManager] ✅ ${message}`);
    }

    logError(message) {
        this.addLogEntry(message, 'error');
        console.error(`[StationManager] ❌ ${message}`);
    }

    addLogEntry(message, type = 'info') {
        const logContainer = document.querySelector('.log-container');
        const logEntry = document.createElement('div');
        logEntry.className = `log-entry log-entry--${type}`;
        
        const time = new Date().toLocaleTimeString();
        logEntry.innerHTML = `
            <span class="log-entry__time">[${time}]</span>
            <span class="log-entry__message">${message}</span>
        `;
        
        logContainer.appendChild(logEntry);
        logContainer.scrollTop = logContainer.scrollHeight;
    }

    handleRpcError(userMessage, error) {
        const errorMessage = error.message || 'Неизвестная ошибка';
        this.logError(`${userMessage}: ${errorMessage}`);
    }
}

// Инициализация при загрузке страницы
document.addEventListener('DOMContentLoaded', () => {
    window.stationManager = new StationManager(window.rpcClient);
});

// Добавьте эти стили в CSS для статуса подключения
const additionalStyles = `
.connection-status {
    position: absolute;
    top: 20px;
    right: 20px;
    padding: 5px 10px;
    border-radius: 15px;
    font-size: 0.8rem;
    font-weight: 500;
}

.connection-status.connected {
    background-color: #2ecc71;
    color: white;
}

.connection-status.disconnected {
    background-color: #e74c3c;
    color: white;
}

.log-entry--success .log-entry__message {
    color: #2ecc71;
}

.log-entry--error .log-entry__message {
    color: #e74c3c;
}

.log-entry--info .log-entry__message {
    color: #3498db;
}
`;

// Добавление стилей в документ
const styleSheet = document.createElement('style');
styleSheet.textContent = additionalStyles;
document.head.appendChild(styleSheet);