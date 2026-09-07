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

    async getWifi() {
        return this.request("/api/wifi");
    },

    async getWifiNetworks() {
        return this.request("/api/wifi/networks");
    },

    async scanWifi() {
        return this.request("/api/wifi/scan");
    },

    async getConfig() {
        return this.request("/api/config");
    },

    async saveConfig(config) {
        return this.request("/api/config", {
            method: "POST",

            headers: {
                "Content-Type": "application/json"
            },

            body: JSON.stringify(config)
        });
    }
};

window.CyberSwissKnifeAPI =
    CyberSwissKnifeAPI;