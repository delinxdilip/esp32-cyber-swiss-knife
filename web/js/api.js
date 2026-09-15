const CyberSwissKnifeAPI = {
    async request(path, options = {}) {
        const response = await fetch(path, {
            cache: "no-store",
            ...options
        });

        if (!response.ok) {
            let message = `HTTP ${response.status}`;

            try {
                const data = await response.json();

                if (data.error) {
                    message = data.error;
                }
            } catch (_) {
                // Ignore non-JSON error responses.
            }

            throw new Error(message);
        }

        return response.json();
    },

    async getState() {
        return this.request("/api/state");
    },

    async getAPConfig() {
        return this.request("/api/config/ap");
    },

    async saveAPConfig(config) {
        return this.request("/api/config/ap", {
            method: "POST",
            headers: {
                "Content-Type": "application/json"
            },
            body: JSON.stringify(config)
        });
    },

    async getTFTConfig() {
        return this.request("/api/config/tft");
    },

    async saveTFTConfig(config) {
        return this.request("/api/config/tft", {
            method: "POST",
            headers: {
                "Content-Type": "application/json"
            },
            body: JSON.stringify(config)
        });
    },

    async getDeviceConfig() {
        return this.request("/api/config/device");
    },

    async saveDeviceConfig(config) {
        return this.request("/api/config/device", {
            method: "POST",
            headers: {
                "Content-Type": "application/json"
            },
            body: JSON.stringify(config)
        });
    },

    async getHardwareConfig() {
        return this.request("/api/config/hardware");
    },

    async saveHardwareConfig(config) {
        return this.request("/api/config/hardware", {
            method: "POST",
            headers: {
                "Content-Type": "application/json"
            },
            body: JSON.stringify(config)
        });
    },

    async getLoggingConfig() {
        return this.request("/api/config/logging");
    },

    async saveLoggingConfig(config) {
        return this.request("/api/config/logging", {
            method: "POST",
            headers: {
                "Content-Type": "application/json"
            },
            body: JSON.stringify(config)
        });
    },

    async getWifi() {
        return this.request("/api/wifi");
    },

    async getWifiAP() {
        return this.request("/api/wifi/ap");
    },

    async getWifiNetworks() {
        return this.request("/api/wifi/networks");
    },

    async scanWifi() {
        return this.request("/api/wifi/scan");
    }
};

window.CyberSwissKnifeAPI = CyberSwissKnifeAPI;