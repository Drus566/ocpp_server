class RPCClient {
	constructor() {
		this.ws = null;
		this.connected = false;
		this.requestId = 0;
		this.pendingRequests = new Map();
		this.eventHandlers = new Map();

		// Автоматическое переподключение
		this.reconnectInterval = 3000;
		this.maxReconnectAttempts = 5;
		this.reconnectAttempts = 0;

		this.url = 'ws://localhost:8080';

		// Отладка
		this.debug = true;
	}

	log(message, data = null) {
		if (this.debug) {
			console.log(`[RPCClient] ${message}`, data || '');
		}
	}

	setUrl(new_url) {
		this.url = new_url;
	}

	connect() {
		try {
			this.log(`Connecting to ${this.url}...`);
			this.ws = new WebSocket(this.url, 'websocket');

			this.ws.onopen = () => {
				this.log('WebSocket connection established');
				this.connected = true;
				this.reconnectAttempts = 0;
				this.emit('connected');
			};

			this.ws.onmessage = (event) => {
				this.log('Message received from server:', event.data);
				try {
					const message = JSON.parse(event.data);
					this.handleMessage(message);
				} catch (error) {
					console.error('Error parsing message:', error, 'Raw data:', event.data);
				}
			};

			this.ws.onclose = () => {
				this.log('WebSocket connection closed');
				this.connected = false;
				this.emit('disconnected');
				this.handleReconnect();
			};

			this.ws.onerror = (error) => {
				this.log('WebSocket error:', error);
				this.emit('error', error);
			};

		} catch (error) {
			console.error('Connection error:', error);
		}
	}

	call(method, params = {}) {
		return new Promise((resolve, reject) => {
			if (!this.connected) {
				const error = new Error('Not connected to server');
				this.log('Call failed - not connected:', method);
				reject(error);
				return;
			}

			const id = ++this.requestId;
			const request = {
				jsonrpc: '2.0',
				id: id,
				method: method,
				params: params
			};

			this.pendingRequests.set(id, { resolve, reject });

			const requestJson = JSON.stringify(request);
			this.log(`Sending RPC request: ${method}`, request);

			try {
				this.ws.send(requestJson);
			} catch (error) {
				this.pendingRequests.delete(id);
				reject(error);
			}
		});
	}

	handleMessage(message) {
		console.log('Raw message received:', message);

		// Если сообщение пришло как строка, пробуем распарсить
		if (typeof message === 'string') {
			try {
				message = JSON.parse(message);
			} catch (e) {
				// Если не JSON, отправляем как строку
				this.emit('message', message);
				return;
			}
		}

		// Ответ на RPC запрос
		if (message.id && this.pendingRequests.has(message.id)) {
			const { resolve, reject } = this.pendingRequests.get(message.id);
			this.pendingRequests.delete(message.id);

			if (message.error) {
				reject(new Error(message.error));
			} else {
				// Возвращаем как есть - вызывающая сторона разберется с двойным JSON
				resolve(message);
			}
		}
		// Уведомление от сервера
		else if (message.method) {
			this.emit(message.method, message.params);
		}
	}
	
	on(event, handler) {
		if (!this.eventHandlers.has(event)) {
			this.eventHandlers.set(event, []);
		}
		this.eventHandlers.get(event).push(handler);
	}

	off(event, handler) {
		if (this.eventHandlers.has(event)) {
			const handlers = this.eventHandlers.get(event);
			const index = handlers.indexOf(handler);
			if (index > -1) {
				handlers.splice(index, 1);
			}
		}
	}

	emit(event, data) {
		if (this.eventHandlers.has(event)) {
			this.eventHandlers.get(event).forEach(handler => {
				try {
					handler(data);
				} catch (error) {
					console.error('Error in event handler:', error);
				}
			});
		}
	}

	handleReconnect() {
		// if (this.reconnectAttempts < this.maxReconnectAttempts) {
		// 	this.reconnectAttempts++;
		// 	console.log(`Attempting to reconnect... (${this.reconnectAttempts}/${this.maxReconnectAttempts})`);

		// 	setTimeout(() => {
		// 		this.connect();
		// 	}, this.reconnectInterval);
		// } else {
		// 	console.error('Max reconnection attempts reached');
		// 	this.emit('reconnect_failed');
		// }
	}

	disconnect() {
		this.connected = false;
		this.reconnectAttempts = this.maxReconnectAttempts; // Prevent auto-reconnect
		if (this.ws) {
			this.ws.close();
		}
	}
}

// Глобальный экземпляр RPC клиента
window.rpc_client = new RPCClient();