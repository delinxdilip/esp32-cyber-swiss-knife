(() => {
    "use strict";


    const elements = {
        statusDot:
            document.getElementById("scanner-status-dot"),

        statusText:
            document.getElementById("scanner-status-text"),

        scanButton:
            document.getElementById("scan-button"),

        scanButtonLabel:
            document.getElementById("scan-button-label"),

        scanStatus:
            document.getElementById("scan-status"),

        summaryCount:
            document.getElementById("summary-count"),

        summaryOpen:
            document.getElementById("summary-open"),

        summaryHidden:
            document.getElementById("summary-hidden"),

        summaryStrongest:
            document.getElementById("summary-strongest"),

        networkListCount:
            document.getElementById("network-list-count"),

        networkList:
            document.getElementById("network-list"),

        scannerEmpty:
            document.getElementById("scanner-empty")
    };


    function setDeviceStatus(online) {
        elements.statusDot.classList.remove(
            "online",
            "offline"
        );

        elements.statusDot.classList.add(
            online ? "online" : "offline"
        );

        elements.statusText.textContent =
            online ? "ONLINE" : "OFFLINE";
    }


    function formatRssi(rssi) {
        if (
            rssi === undefined ||
            rssi === null
        ) {
            return "—";
        }

        return `${rssi} dBm`;
    }


    function getSignalLevel(rssi) {
        if (rssi >= -50) {
            return 4;
        }

        if (rssi >= -60) {
            return 3;
        }

        if (rssi >= -70) {
            return 2;
        }

        if (rssi >= -80) {
            return 1;
        }

        return 0;
    }


    function createSignalBars(rssi) {
        const level =
            getSignalLevel(rssi);

        const wrapper =
            document.createElement("span");

        wrapper.className = "signal-bars";

        for (let index = 1; index <= 4; index += 1) {
            const bar =
                document.createElement("i");

            if (index <= level) {
                bar.style.opacity = "1";
            } else {
                bar.style.opacity = ".2";
            }

            wrapper.appendChild(bar);
        }

        return wrapper;
    }


    function createElement(tag, className, text) {
        const element =
            document.createElement(tag);

        if (className) {
            element.className = className;
        }

        if (text !== undefined) {
            element.textContent = text;
        }

        return element;
    }


    function createNetworkRow(network) {
        const row =
            createElement("article", "network-row");


        /* SSID / BSSID */

        const primary =
            createElement("div", "network-primary");

        const ssid =
            createElement(
                "strong",
                "network-ssid",
                network.ssid || "Hidden network"
            );

        const bssid =
            createElement(
                "span",
                "network-bssid",
                network.bssid || "—"
            );

        primary.appendChild(ssid);
        primary.appendChild(bssid);


        /* Signal */

        const signalField =
            createElement("div", "network-field");

        const signalLabel =
            createElement(
                "span",
                null,
                "SIGNAL"
            );

        const signalValue =
            createElement(
                "strong",
                "signal-value"
            );

        signalValue.appendChild(
            createSignalBars(network.rssi)
        );

        signalValue.appendChild(
            document.createTextNode(
                formatRssi(network.rssi)
            )
        );

        signalField.appendChild(signalLabel);
        signalField.appendChild(signalValue);


        /* Channel */

        const channelField =
            createElement("div", "network-field");

        channelField.appendChild(
            createElement(
                "span",
                null,
                "CHANNEL"
            )
        );

        channelField.appendChild(
            createElement(
                "strong",
                null,
                network.channel ?? "—"
            )
        );


        /* Hidden */

        const hiddenField =
            createElement("div", "network-field");

        hiddenField.appendChild(
            createElement(
                "span",
                null,
                "HIDDEN"
            )
        );

        hiddenField.appendChild(
            createElement(
                "strong",
                null,
                network.hidden ? "YES" : "NO"
            )
        );


        /* Append */

        row.appendChild(primary);
        row.appendChild(signalField);
        row.appendChild(channelField);
        row.appendChild(hiddenField);

        return row;
    }


    function renderNetworks(networks) {
        elements.networkList.innerHTML = "";

        if (
            !Array.isArray(networks) ||
            networks.length === 0
        ) {
            elements.networkList.appendChild(
                elements.scannerEmpty
            );

            elements.networkListCount.textContent =
                "0 networks";

            return;
        }


        elements.networkListCount.textContent =
            `${networks.length} ${
                networks.length === 1
                    ? "network"
                    : "networks"
            }`;


        networks.forEach((network) => {
            elements.networkList.appendChild(
                createNetworkRow(network)
            );
        });
    }


    function updateSummary(data) {
        if (!data) {
            return;
        }

        elements.summaryCount.textContent =
            data.network_count ?? "—";

        elements.summaryOpen.textContent =
            data.open_network_count ?? "—";

        elements.summaryHidden.textContent =
            data.hidden_network_count ?? "—";

        if (
            data.strongest_rssi === undefined ||
            data.strongest_rssi === null
        ) {
            elements.summaryStrongest.textContent =
                "—";
        } else {
            elements.summaryStrongest.textContent =
                formatRssi(data.strongest_rssi);
        }
    }


    async function loadSummary() {
        const data =
            await CyberSwissKnifeAPI.getWifi();

        updateSummary(data);
    }


    async function loadNetworks() {
        const data =
            await CyberSwissKnifeAPI.getWifiNetworks();

        renderNetworks(data.networks);
    }


    async function loadResults() {
        try {
            await Promise.all([
                loadSummary(),
                loadNetworks()
            ]);

            setDeviceStatus(true);

        } catch (error) {
            console.error(
                "Unable to load Wi-Fi scanner data:",
                error
            );

            setDeviceStatus(false);

            elements.scanStatus.textContent =
                "Unable to load scanner data.";
        }
    }


    async function performScan() {
        if (elements.scanButton.disabled) {
            return;
        }


        elements.scanButton.disabled = true;

        elements.scanButtonLabel.textContent =
            "Scanning…";

        elements.scanStatus.textContent =
            "Scanning nearby wireless networks…";


        try {
            /*
             * The scan endpoint performs the actual
             * ESP32 Wi-Fi scan.
             *
             * After it completes, fetch the stored
             * scanner results separately.
             */
            await CyberSwissKnifeAPI.scanWifi();

            elements.scanStatus.textContent =
                "Scan complete. Updating results…";


            await loadResults();


            elements.scanStatus.textContent =
                "Scan complete.";

        } catch (error) {
            console.error(
                "Wi-Fi scan failed:",
                error
            );

            setDeviceStatus(false);

            elements.scanStatus.textContent =
                `Scan failed: ${error.message}`;
        } finally {
            elements.scanButton.disabled = false;

            elements.scanButtonLabel.textContent =
                "Scan Networks";
        }
    }


    elements.scanButton.addEventListener(
        "click",
        performScan
    );


    loadResults();
})();