class AdminPanel {
	constructor() {
		this.init();
	}

	init() {
		rpcClient.on('connected', () => {
			this.updateConnectionStatus(true);
			this.loadSystemInfo();
		});

		rpcClient.on('disconnected', () => {
			this.updateConnectionStatus(false);
		});

		rpcClient.connect();
		this.setupEventHandlers();
	}

	async loadSystemInfo() {
		try {
			const values = await rpcClient.call('getValues');
			this.displaySystemInfo(values);
		} catch (error) {
			console.error('Failed to load system info:', error);
		}
	}

	displaySystemInfo(values) {
		const infoContainer = document.querySelector('.admin-panel');
		let html = `
            <div class="admin-section">
                <h2>System Configuration</h2>
                <div class="config-grid">
        `;

		for (const [key, value] of Object.entries(values)) {
			html += `
                <div class="config-item">
                    <label>${key}:</label>
                    <span class="config-value">${value}</span>
                    <button onclick="adminPanel.editValue('${key}', '${value}')" 
                            class="btn btn-small">Edit</button>
                </div>
            `;
		}

		html += `
                </div>
            </div>
            <div class="admin-section">
                <h2>System Commands</h2>
                <div class="command-panel">
                    <button onclick="adminPanel.rebootSystem()" class="btn btn-warning">Reboot System</button>
                    <button onclick="adminPanel.clearLogs()" class="btn btn-secondary">Clear Logs</button>
                    <button onclick="adminPanel.exportConfig()" class="btn btn-primary">Export Config</button>
                </div>
            </div>
        `;

		infoContainer.innerHTML = html;
	}

	editValue(key, currentValue) {
		const newValue = prompt(`Edit ${key}:`, currentValue);
		if (newValue !== null) {
			this.setValue(key, newValue);
		}
	}

	async setValue(key, value) {
		try {
			await rpcClient.call('setValue', { key, value });
			this.addLog(`Configuration updated: ${key} = ${value}`);
		} catch (error) {
			alert(`Error setting value: ${error.message}`);
		}
	}

	async rebootSystem() {
		if (confirm('Are you sure you want to reboot the system?')) {
			try {
				await rpcClient.call('resetSystem');
				this.addLog('System reboot initiated');
			} catch (error) {
				alert(`Error rebooting system: ${error.message}`);
			}
		}
	}

	clearLogs() {
		// Implementation for clearing logs
		this.addLog('Logs cleared');
	}

	exportConfig() {
		// Implementation for exporting configuration
		this.addLog('Configuration exported');
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

	addLog(message) {
		console.log(`[Admin] ${message}`);
		// You can add a log display in the admin panel if needed
	}

	setupEventHandlers() {
		// Add any additional event handlers here
	}
}

// Initialize admin panel when page loads
document.addEventListener('DOMContentLoaded', () => {
	window.adminPanel = new AdminPanel();
});