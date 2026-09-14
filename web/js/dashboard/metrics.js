const DashboardMetrics = {
    setStatus(online) {
        const dot =
            document.getElementById("status-dot");

        const text =
            document.getElementById("status-text");

        const detail =
            document.getElementById("connection-detail");

        if (!dot || !text) {
            return;
        }

        dot.classList.toggle("online", online);
        dot.classList.toggle("offline", !online);

        text.textContent =
            online
                ? "ONLINE"
                : "OFFLINE";

        if (detail) {
            detail.textContent =
                online
                    ? "ESP32-S3 connected"
                    : "ESP32-S3 unavailable";
        }
    },

    setStatusValue(elementId, status, text) {
        const element =
            document.getElementById(elementId);

        if (!element) {
            return;
        }

        element.classList.remove(
            "connected",
            "enabled",
            "disabled");

        if (status === "CONN") {
            element.classList.add("connected");
        } else if (status === "ON") {
            element.classList.add("enabled");
        } else {
            element.classList.add("disabled");
        }

        const indicator = element.querySelector("i");

        element.textContent = "";

        if (indicator) {
            element.appendChild(indicator);
        }

        element.append(` ${text}`);
    },

    formatUptime(seconds) {
        const totalSeconds =
            Math.max(0, Number(seconds) || 0);

        const days =
            Math.floor(totalSeconds / 86400);

        const hours =
            Math.floor((totalSeconds % 86400) / 3600);

        const minutes =
            Math.floor((totalSeconds % 3600) / 60);

        const remainingSeconds =
            Math.floor(totalSeconds % 60);

        const time =
            [
                hours,
                minutes,
                remainingSeconds
            ]
                .map(value =>
                    String(value).padStart(2, "0"))
                .join(":");

        return days > 0
            ? `${days}D ${time}`
            : time;
    },

    updateState(data) {
        if (!data) {
            return;
        }

        window.cyberSwissKnifeState = {
            ...(window.cyberSwissKnifeState || {}),
            state: data
        };

        const temperature =
            document.getElementById(
                "system-temperature");

        const uptime =
            document.getElementById(
                "system-uptime");

        if (temperature &&
            Number.isFinite(Number(data.temperature))) {
            temperature.textContent =
                `${Number(data.temperature).toFixed(1)}°C`;
        }

        if (uptime) {
            uptime.textContent =
                this.formatUptime(data.uptime);
        }

        this.setStatusValue(
            "system-wifi-status",
            data.wifi?.status || "OFF",
            data.wifi?.status || "OFF");

        this.setStatusValue(
            "system-bluetooth-status",
            data.bluetooth?.status || "OFF",
            data.bluetooth?.status || "OFF");

        const apRunning =
            data.ap?.status === "ON";

        const clients =
            Number(data.ap?.clients) || 0;

        const maxClients =
            Number(data.ap?.max_clients) || 0;

        this.setStatusValue(
            "system-ap-status",
            apRunning ? "CONN" : "OFF",
            `${String(clients).padStart(2, "0")}/${String(maxClients).padStart(2, "0")}`);
    }
};

window.DashboardMetrics = DashboardMetrics;