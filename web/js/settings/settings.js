document.addEventListener("DOMContentLoaded", async () => {
    const get = (id) =>
        document.getElementById(id);

    function showStatus(
        element,
        message,
        type = "info"
    ) {
        if (!element) {
            return;
        }

        element.textContent = message;
        element.dataset.type = type;
    }

    function setSaving(
        button,
        saving,
        label
    ) {
        if (!button) {
            return;
        }

        button.disabled = saving;

        button.textContent = saving
            ? "Saving..."
            : label;
    }

    function colorToHex(value) {
        const color =
            Number(value) >>> 0;

        return `#${color
            .toString(16)
            .padStart(6, "0")
            .slice(-6)}`;
    }

    function hexToColor(value) {
        return parseInt(
            value.replace("#", ""),
            16);
    }

    async function saveSection(
        button,
        messageElement,
        label,
        saveRequest,
        config,
        successMessage
    ) {
        setSaving(button, true, label);
        showStatus(
            messageElement,
            "Saving configuration..."
        );

        try {
            const result =
                await saveRequest(config);

            if (result?.success === false) {
                throw new Error(
                    result.error ||
                    "Failed to save configuration."
                );
            }

            showStatus(
                messageElement,
                successMessage,
                "success"
            );
        } catch (error) {
            console.error(error);

            showStatus(
                messageElement,
                `Failed to save: ${error.message}`,
                "error"
            );
        } finally {
            setSaving(button, false, label);
        }
    }

    const apForm =
        get("config-form");

    const apStatus =
        get("form-message");

    const apSaveButton =
        apForm?.querySelector(
            'button[type="submit"]');

    const ssidInput = get("ssid");
    const passwordInput = get("password");
    const maxConnectionsInput =
        get("max-connections");
    const channelInput = get("channel");

    function validateAP(config) {
        if (
            config.ssid.length < 1 ||
            config.ssid.length > 32
        ) {
            return "SSID must be between 1 and 32 characters.";
        }

        if (
            config.password.length !== 0 &&
            (
                config.password.length < 8 ||
                config.password.length > 64
            )
        ) {
            return "Password must be empty or between 8 and 64 characters.";
        }

        if (
            !Number.isInteger(
                config.max_connections) ||
            config.max_connections < 1 ||
            config.max_connections > 10
        ) {
            return "Maximum connections must be between 1 and 10.";
        }

        if (
            !Number.isInteger(
                config.channel) ||
            config.channel < 1 ||
            config.channel > 13
        ) {
            return "Channel must be between 1 and 13.";
        }

        return null;
    }

    async function loadAPConfig() {
        showStatus(
            apStatus,
            "Loading current configuration..."
        );

        try {
            const config =
                await CyberSwissKnifeAPI.getAPConfig();

            ssidInput.value = config.ssid;
            maxConnectionsInput.value =
                config.max_connections;
            channelInput.value =
                config.channel;

            passwordInput.value = "";

            showStatus(
                apStatus,
                "Configuration loaded.",
                "success"
            );
        } catch (error) {
            console.error(error);

            showStatus(
                apStatus,
                `Unable to load configuration: ${error.message}`,
                "error"
            );
        }
    }

    apForm?.addEventListener(
        "submit",
        async (event) => {
            event.preventDefault();

            const config = {
                ssid:
                    ssidInput.value.trim(),

                password:
                    passwordInput.value,

                max_connections:
                    Number(
                        maxConnectionsInput.value),

                channel:
                    Number(channelInput.value)
            };

            const error =
                validateAP(config);

            if (error) {
                showStatus(
                    apStatus,
                    error,
                    "error"
                );

                return;
            }

            if (!window.confirm(
                "Applying these changes may disconnect your current device from CyberSwissKnife. Continue?"
            )) {
                return;
            }

            await saveSection(
                apSaveButton,
                apStatus,
                "Save Access Point",
                CyberSwissKnifeAPI.saveAPConfig.bind(
                    CyberSwissKnifeAPI),
                config,
                "Configuration saved. The access point may disconnect this device."
            );
        }
    );

    const tftStatus =
        get("tft-message");

    const tftSaveButton =
        get("save-tft-config");

    async function loadTFTConfig() {
        try {
            const config =
                await CyberSwissKnifeAPI.getTFTConfig();

            get("tft-enabled").checked =
                config.enabled;

            get("tft-controller").value =
                config.controller;

            get("tft-shape").value =
                config.shape;

            get("tft-width").value =
                config.width;

            get("tft-height").value =
                config.height;

            get("tft-rotation").value =
                config.rotation;

            get("tft-touch").checked =
                config.touch;

            get("tft-color").value =
                colorToHex(config.color);

            showStatus(
                tftStatus,
                "TFT configuration loaded.",
                "success"
            );
        } catch (error) {
            showStatus(
                tftStatus,
                `Unable to load TFT configuration: ${error.message}`,
                "error"
            );
        }
    }

    tftSaveButton?.addEventListener(
        "click",
        async () => {
            const config = {
                enabled:
                    get("tft-enabled").checked,

                controller:
                    Number(
                        get("tft-controller").value),

                shape:
                    Number(
                        get("tft-shape").value),

                width:
                    Number(get("tft-width").value),

                height:
                    Number(get("tft-height").value),

                rotation:
                    Number(
                        get("tft-rotation").value),

                touch:
                    get("tft-touch").checked,

                color:
                    hexToColor(
                        get("tft-color").value)
            };

            await saveSection(
                tftSaveButton,
                tftStatus,
                "Save TFT Display",
                CyberSwissKnifeAPI.saveTFTConfig.bind(
                    CyberSwissKnifeAPI),
                config,
                "TFT configuration saved. Restart the device to apply display changes."
            );
        }
    );

    const deviceStatus =
        get("device-message");

    const deviceSaveButton =
        get("save-device-config");

    async function loadDeviceConfig() {
        try {
            const config =
                await CyberSwissKnifeAPI.getDeviceConfig();

            get("device-name").value =
                config.device_name;

            get("web-ui-title").value =
                config.web_ui_title;

            get("ap-identification").checked =
                config.ap_identification;

            get("device-discovery").checked =
                config.device_discovery;

            get("bluetooth-name").value =
                config.bluetooth_name;

            get("theme-color").value =
                colorToHex(
                    config.tft_color_theme);

            showStatus(
                deviceStatus,
                "Device configuration loaded.",
                "success"
            );
        } catch (error) {
            showStatus(
                deviceStatus,
                `Unable to load device configuration: ${error.message}`,
                "error"
            );
        }
    }

    deviceSaveButton?.addEventListener(
        "click",
        async () => {
            const config = {
                device_name:
                    get("device-name")
                        .value.trim(),

                web_ui_title:
                    get("web-ui-title")
                        .value.trim(),

                ap_identification:
                    get("ap-identification")
                        .checked,

                device_discovery:
                    get("device-discovery")
                        .checked,

                bluetooth_name:
                    get("bluetooth-name")
                        .value.trim(),

                tft_color_theme:
                    hexToColor(
                        get("theme-color").value)
            };

            if (
                config.device_name.length < 1 ||
                config.web_ui_title.length < 1 ||
                config.bluetooth_name.length < 1
            ) {
                showStatus(
                    deviceStatus,
                    "Device name, Web UI title, and Bluetooth name are required.",
                    "error"
                );

                return;
            }

            await saveSection(
                deviceSaveButton,
                deviceStatus,
                "Save Device Settings",
                CyberSwissKnifeAPI.saveDeviceConfig.bind(
                    CyberSwissKnifeAPI),
                config,
                "Device configuration saved. Restart the device to apply identity changes."
            );
        }
    );

    const hardwareStatus =
        get("hardware-message");

    const hardwareSaveButton =
        get("save-hardware-config");

    async function loadHardwareConfig() {
        try {
            const config =
                await CyberSwissKnifeAPI.getHardwareConfig();

            get("rgb-led-enabled").checked =
                config.onboard_rgb_led_enabled;

            get("rgb-led-pin").value =
                config.onboard_rgb_pin;

            showStatus(
                hardwareStatus,
                "Hardware configuration loaded.",
                "success"
            );
        } catch (error) {
            showStatus(
                hardwareStatus,
                `Unable to load hardware configuration: ${error.message}`,
                "error"
            );
        }
    }

    hardwareSaveButton?.addEventListener(
        "click",
        async () => {
            const config = {
                onboard_rgb_led_enabled:
                    get("rgb-led-enabled").checked,

                onboard_rgb_pin:
                    Number(
                        get("rgb-led-pin").value)
            };

            if (
                !Number.isInteger(
                    config.onboard_rgb_pin) ||
                config.onboard_rgb_pin < 0 ||
                config.onboard_rgb_pin > 48
            ) {
                showStatus(
                    hardwareStatus,
                    "RGB LED GPIO must be between 0 and 48.",
                    "error"
                );

                return;
            }

            await saveSection(
                hardwareSaveButton,
                hardwareStatus,
                "Save Hardware Settings",
                CyberSwissKnifeAPI.saveHardwareConfig.bind(
                    CyberSwissKnifeAPI),
                config,
                "Hardware configuration saved. Restart the device to apply GPIO changes."
            );
        }
    );

    const loggingStatus =
        get("logging-message");

    const loggingSaveButton =
        get("save-logging-config");

    async function loadLoggingConfig() {
        try {
            const config =
                await CyberSwissKnifeAPI.getLoggingConfig();

            get("system-auto-clear").checked =
                config.system_auto_clear;

            get("activity-auto-clear").checked =
                config.activity_auto_clear;

            showStatus(
                loggingStatus,
                "Logging configuration loaded.",
                "success"
            );
        } catch (error) {
            showStatus(
                loggingStatus,
                `Unable to load logging configuration: ${error.message}`,
                "error"
            );
        }
    }

    loggingSaveButton?.addEventListener(
        "click",
        async () => {
            const config = {
                system_auto_clear:
                    get("system-auto-clear").checked,

                activity_auto_clear:
                    get("activity-auto-clear").checked
            };

            await saveSection(
                loggingSaveButton,
                loggingStatus,
                "Save Logging Settings",
                CyberSwissKnifeAPI.saveLoggingConfig.bind(
                    CyberSwissKnifeAPI),
                config,
                "Logging configuration saved."
            );
        }
    );

    await Promise.all([
        loadAPConfig(),
        loadTFTConfig(),
        loadDeviceConfig(),
        loadHardwareConfig(),
        loadLoggingConfig()
    ]);
});