const SystemInfoPage = {
    formatBytes(bytes) {
        if (!Number.isFinite(bytes)) {
            return "—";
        }

        if (bytes < 1024) {
            return `${bytes} B`;
        }

        if (bytes < 1024 * 1024) {
            return `${(bytes / 1024).toFixed(1)} KB`;
        }

        if (bytes < 1024 * 1024 * 1024) {
            return `${(bytes / (1024 * 1024)).toFixed(2)} MB`;
        }

        return `${(bytes / (1024 * 1024 * 1024)).toFixed(2)} GB`;
    },

    formatUptime(seconds) {
        if (!Number.isFinite(seconds)) {
            return "—";
        }

        seconds = Math.floor(seconds);

        const days = Math.floor(seconds / 86400);

        seconds %= 86400;

        const hours = Math.floor(seconds / 3600);

        seconds %= 3600;

        const minutes = Math.floor(seconds / 60);

        const secs = seconds % 60;

        if (days > 0) {
            return `${days}d ${String(hours).padStart(2, "0")}:${String(minutes).padStart(2, "0")}:${String(secs).padStart(2, "0")}`;
        }

        return `${String(hours).padStart(2, "0")}:${String(minutes).padStart(2, "0")}:${String(secs).padStart(2, "0")}`;
    },

    set(id, value) {
        const element = document.getElementById(id);

        if (element) {
            element.textContent =
                value === undefined ||
                value === null ||
                value === ""
                    ? "—"
                    : value;
        }
    },

    async refresh() {
        try {
            const data =
                await CyberSwissKnifeAPI.request(
                    "/api/system"
                );

            this.render(data);

            this.set(
                "system-status",
                "ONLINE"
            );

        } catch (error) {

            console.error(
                "System information error:",
                error
            );

            this.set(
                "system-status",
                "OFFLINE"
            );
        }
    },

    render(data) {
        const device = data.device || {};
        const memory = data.memory || {};
        const flash = data.flash || {};
        const health = data.health || {};
        const network = data.network || {};
        const storage = data.storage || {};
        const firmware = data.firmware || {};

        /*
         * Device
         */

        this.set(
            "device-name",
            device.name
        );

        this.set(
            "device-chip",
            device.chip
        );

        this.set(
            "device-revision",
            `Rev ${device.chip_revision}`
        );

        this.set(
            "device-cores",
            device.cpu_cores
        );

        this.set(
            "device-frequency",
            `${device.cpu_frequency_mhz} MHz`
        );

        this.set(
            "device-idf",
            device.esp_idf_version
        );

        this.set(
            "device-firmware",
            `v${device.firmware_version}`
        );

        this.set(
            "device-build",
            device.build_date
        );

        this.set(
            "device-reset",
            device.reset_reason
        );

        this.set(
            "device-uptime",
            this.formatUptime(
                device.uptime_seconds
            )
        );

        /*
         * Memory
         */

        this.set(
            "memory-free",
            this.formatBytes(
                memory.free_internal_ram
            )
        );

        this.set(
            "memory-total",
            this.formatBytes(
                memory.total_internal_ram
            )
        );

        this.set(
            "memory-used",
            this.formatBytes(
                memory.used_internal_ram
            )
        );

        this.set(
            "memory-min",
            this.formatBytes(
                memory.minimum_free_heap
            )
        );

        this.set(
            "memory-psram-total",
            memory.psram_total > 0
                ? this.formatBytes(
                    memory.psram_total
                )
                : "Not detected"
        );

        this.set(
            "memory-psram-free",
            memory.psram_total > 0
                ? this.formatBytes(
                    memory.psram_free
                )
                : "Not detected"
        );

        /*
         * Flash
         */

        this.set(
            "flash-size",
            this.formatBytes(
                flash.size
            )
        );

        this.set(
            "flash-speed",
            `${flash.speed_mhz} MHz`
        );

        this.set(
            "flash-mode",
            flash.mode
        );

        this.set(
            "flash-app",
            this.formatBytes(
                flash.application_partition
            )
        );

        this.set(
            "flash-spiffs",
            this.formatBytes(
                flash.spiffs_partition
            )
        );

        this.set(
            "flash-used",
            this.formatBytes(
                flash.spiffs_used
            )
        );

        this.set(
            "flash-free",
            this.formatBytes(
                flash.spiffs_free
            )
        );

        /*
         * Health
         */

        this.set(
            "health-temperature",
            `${Number(
                health.temperature_celsius
            ).toFixed(1)} °C`
        );

        this.set(
            "health-cpu-load",
            `${Number(
                health.cpu_load_percent
            ).toFixed(1)}%`
        );

        this.set(
            "health-cores",
            health.cpu_cores
        );

        this.set(
            "health-revision",
            `Rev ${health.chip_revision}`
        );

        this.set(
            "health-wifi",
            health.wifi_state
        );

        this.set(
            "health-bluetooth",
            health.bluetooth_state
        );

        this.set(
            "health-ap",
            health.ap_state
        );

        this.set(
            "health-clients",
            health.connected_clients
        );

        this.set(
            "health-tasks",
            health.task_count
        );

        /*
         * Network
         */

        this.set(
            "network-wifi",
            network.wifi_status
        );

        this.set(
            "network-ap-status",
            network.ap_status
        );

        this.set(
            "network-ssid",
            network.ssid
        );

        this.set(
            "network-ip",
            network.ip_address
        );

        this.set(
            "network-channel",
            network.channel
        );

        this.set(
            "network-clients",
            network.clients
        );

        this.set(
            "network-mac",
            network.mac_address
        );

        /*
         * Storage
         */

        this.set(
            "storage-used",
            this.formatBytes(
                storage.used
            )
        );

        this.set(
            "storage-total",
            this.formatBytes(
                storage.total
            )
        );

        this.set(
            "storage-free",
            this.formatBytes(
                storage.free
            )
        );

        this.set(
            "storage-files",
            storage.files
        );

        this.set(
            "storage-logs",
            this.formatBytes(
                storage.logs
            )
        );

        this.set(
            "storage-web",
            this.formatBytes(
                storage.web_ui
            )
        );

        this.set(
            "storage-config",
            this.formatBytes(
                storage.configuration
            )
        );

        const total =
            Number(storage.total) || 0;

        const used =
            Number(storage.used) || 0;

        const percentage =
            total > 0
                ? Math.min(
                    100,
                    (used / total) * 100
                )
                : 0;

        const progress =
            document.getElementById(
                "storage-progress"
            );

        if (progress) {
            progress.style.width =
                `${percentage}%`;
        }

        /*
         * Firmware
         */

        this.set(
            "firmware-name",
            firmware.name
        );

        this.set(
            "firmware-version",
            `v${firmware.version}`
        );

        this.set(
            "firmware-date",
            firmware.build_date
        );

        this.set(
            "firmware-time",
            firmware.build_time
        );

        this.set(
            "firmware-target",
            firmware.target
        );

        this.set(
            "firmware-framework",
            firmware.framework
        );

        this.set(
            "firmware-idf",
            firmware.idf_version
        );
    },

    init() {
        this.refresh();

        setInterval(
            () => {
                this.refresh();
            },
            5000
        );
    }
};

document.addEventListener(
    "DOMContentLoaded",
    () => {
        SystemInfoPage.init();
    }
);