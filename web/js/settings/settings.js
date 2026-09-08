document.addEventListener("DOMContentLoaded", async () => {

    const form =
        document.getElementById("config-form");

    const ssidInput =
        document.getElementById("ssid");

    const passwordInput =
        document.getElementById("password");

    const maxConnectionsInput =
        document.getElementById("max-connections");

    const channelInput =
        document.getElementById("channel");

    const statusElement =
        document.getElementById("form-message");

    const saveButton =
        form.querySelector(
            'button[type="submit"]'
        );


    function showStatus(
        message,
        type = "info"
    ) {

        if (!statusElement) {
            return;
        }

        statusElement.textContent =
            message;

        statusElement.dataset.type =
            type;
    }


    function setSaving(saving) {

        if (!saveButton) {
            return;
        }

        saveButton.disabled =
            saving;

        saveButton.textContent =
            saving
                ? "Saving..."
                : "Save Configuration";
    }


    function validate(config) {

        const ssidLength =
            config.ssid.length;

        const passwordLength =
            config.password.length;


        if (
            ssidLength < 1 ||
            ssidLength > 32
        ) {

            return (
                "SSID must be between " +
                "1 and 32 characters."
            );
        }


        /*
         * Password is optional in the UI.
         *
         * An empty password means:
         * keep the currently configured
         * password.
         */

        if (
            passwordLength !== 0 &&
            (
                passwordLength < 8 ||
                passwordLength > 64
            )
        ) {

            return (
                "Password must be empty or " +
                "between 8 and 64 characters."
            );
        }


        if (
            !Number.isInteger(
                config.max_connections
            ) ||
            config.max_connections < 1 ||
            config.max_connections > 10
        ) {

            return (
                "Maximum connections must " +
                "be between 1 and 10."
            );
        }


        if (
            !Number.isInteger(
                config.channel
            ) ||
            config.channel < 1 ||
            config.channel > 13
        ) {

            return (
                "Channel must be between 1 and 13."
            );
        }


        return null;
    }


    async function loadConfig() {

        showStatus(
            "Loading current configuration..."
        );


        try {

            const config =
                await CyberSwissKnifeAPI.getConfig();


            /*
             * GET /api/config currently returns:
             *
             * {
             *     "ssid": "CyberSwissKnife",
             *     "max_connections": 4,
             *     "channel": 1
             * }
             *
             * The firmware intentionally does not
             * return a "success" field here.
             *
             * Therefore, validate the actual fields
             * instead of requiring config.success.
             */

            if (
                !config ||
                typeof config !== "object"
            ) {

                throw new Error(
                    "Invalid configuration response."
                );
            }


            if (
                typeof config.ssid !== "string"
            ) {

                throw new Error(
                    "Configuration response is missing SSID."
                );
            }


            if (
                !Number.isInteger(
                    config.max_connections
                )
            ) {

                throw new Error(
                    "Configuration response is missing maximum connections."
                );
            }


            if (
                !Number.isInteger(
                    config.channel
                )
            ) {

                throw new Error(
                    "Configuration response is missing Wi-Fi channel."
                );
            }


            ssidInput.value =
                config.ssid;


            maxConnectionsInput.value =
                config.max_connections;


            channelInput.value =
                config.channel;


            /*
             * The firmware intentionally does not
             * return the stored password.
             *
             * Keep the password field empty.
             */

            passwordInput.value = "";

            passwordInput.placeholder =
                "Leave blank to keep current password";


            showStatus(
                "Configuration loaded.",
                "success"
            );

        } catch (error) {

            console.error(error);

            showStatus(
                `Unable to load configuration: ${error.message}`,
                "error"
            );
        }
    }


    form.addEventListener(
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
                        maxConnectionsInput.value
                    ),

                channel:
                    Number(
                        channelInput.value
                    )
            };


            const validationError =
                validate(config);


            if (validationError) {

                showStatus(
                    validationError,
                    "error"
                );

                return;
            }


            const confirmed =
                window.confirm(
                    "Applying these changes may disconnect your current device from CyberSwissKnife. Continue?"
                );


            if (!confirmed) {
                return;
            }


            setSaving(true);


            showStatus(
                "Applying configuration..."
            );


            try {

                const result =
                    await CyberSwissKnifeAPI.saveConfig(
                        config
                    );


                /*
                 * The POST endpoint returns a success
                 * response from the firmware.
                 *
                 * Accept a normal successful response
                 * as well as an explicit success:false
                 * error response.
                 */

                if (
                    result &&
                    result.success === false
                ) {

                    throw new Error(
                        result.error ||
                        "Failed to save configuration."
                    );
                }


                showStatus(
                    "Configuration saved. The access point may disconnect this device.",
                    "success"
                );


                /*
                 * If the SSID, password, or channel
                 * changed, the browser may lose its
                 * connection to the ESP32.
                 *
                 * Do not immediately make another API
                 * request because the AP may already
                 * be unavailable.
                 */

            } catch (error) {

                console.error(error);


                /*
                 * If the AP changed successfully,
                 * the browser can report a network
                 * error even though the ESP32 applied
                 * the configuration.
                 */

                showStatus(
                    `Configuration request failed: ${error.message}`,
                    "error"
                );

            } finally {

                setSaving(false);
            }
        }
    );


    await loadConfig();
});