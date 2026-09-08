(() => {
    "use strict";

    const elements = {
        statusDot:
            document.getElementById("wifi-status-dot"),

        statusText:
            document.getElementById("wifi-status-text"),

        apSsid:
            document.getElementById("ap-ssid"),

        apStatus:
            document.getElementById("ap-status"),

        apIp:
            document.getElementById("ap-ip"),

        apChannel:
            document.getElementById("ap-channel"),

        apClients:
            document.getElementById("ap-clients"),

        networkCount:
            document.getElementById("network-count"),

        openCount:
            document.getElementById("open-count"),

        hiddenCount:
            document.getElementById("hidden-count"),

        strongestRssi:
            document.getElementById("strongest-rssi"),

        weakestRssi:
            document.getElementById("weakest-rssi"),

        latestNetworkName:
            document.getElementById("latest-network-name"),

        latestNetworkDetails:
            document.getElementById("latest-network-details"),

        latestNetworkRssi:
            document.getElementById("latest-network-rssi"),

        latestNetworkChannel:
            document.getElementById("latest-network-channel"),

        latestNetworkBssid:
            document.getElementById("latest-network-bssid")
    };

    function setStatus(online) {
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

    function valueOrDash(value) {
        if (
            value === undefined ||
            value === null ||
            value === ""
        ) {
            return "—";
        }

        return value;
    }

    function formatRssi(value) {
        if (
            value === undefined ||
            value === null
        ) {
            return "—";
        }

        return `${value} dBm`;
    }

    function updateAccessPoint(data) {
        if (!data) {
            elements.apSsid.textContent = "—";

            elements.apStatus.textContent =
                "Access point information unavailable.";

            elements.apIp.textContent = "—";
            elements.apChannel.textContent = "—";
            elements.apClients.textContent = "—";

            return;
        }

        elements.apSsid.textContent =
            valueOrDash(data.ssid);

        if (data.status === "running") {
            elements.apStatus.textContent =
                "Access point is running.";
        } else {
            elements.apStatus.textContent =
                valueOrDash(data.status);
        }

        elements.apIp.textContent =
            valueOrDash(data.ip_address);

        elements.apChannel.textContent =
            valueOrDash(data.channel);

        elements.apClients.textContent =
            data.clients !== undefined &&
            data.max_connections !== undefined
                ? `${data.clients} / ${data.max_connections}`
                : valueOrDash(data.clients);
    }

    function updateScanSummary(data) {
        elements.networkCount.textContent =
            valueOrDash(data.network_count);

        elements.openCount.textContent =
            valueOrDash(data.open_network_count);

        elements.hiddenCount.textContent =
            valueOrDash(data.hidden_network_count);

        elements.strongestRssi.textContent =
            formatRssi(data.strongest_rssi);

        elements.weakestRssi.textContent =
            formatRssi(data.weakest_rssi);
    }

    function updateLatestNetwork(data) {
        if (
            !data ||
            !Array.isArray(data.networks) ||
            data.networks.length === 0
        ) {
            elements.latestNetworkName.textContent =
                "—";

            elements.latestNetworkDetails.textContent =
                "No scanned networks available.";

            elements.latestNetworkRssi.textContent =
                "—";

            elements.latestNetworkChannel.textContent =
                "—";

            elements.latestNetworkBssid.textContent =
                "—";

            return;
        }

        const network = data.networks[0];

        elements.latestNetworkName.textContent =
            valueOrDash(network.ssid);

        elements.latestNetworkDetails.textContent =
            network.hidden
                ? "Hidden wireless network"
                : "Wireless network discovered during scan.";

        elements.latestNetworkRssi.textContent =
            formatRssi(network.rssi);

        elements.latestNetworkChannel.textContent =
            valueOrDash(network.channel);

        elements.latestNetworkBssid.textContent =
            valueOrDash(network.bssid);
    }

    async function loadAccessPoint() {
        try {
            const ap =
                await CyberSwissKnifeAPI.getWifiAP();

            updateAccessPoint(ap);

            return true;
        } catch (error) {
            console.warn(
                "Unable to load access point information:",
                error
            );

            updateAccessPoint(null);

            return false;
        }
    }

    async function loadDashboard() {
        const apOnline =
            await loadAccessPoint();

        try {
            const wifi =
                await CyberSwissKnifeAPI.getWifi();

            setStatus(apOnline);
            updateScanSummary(wifi);
        } catch (error) {
            console.error(
                "Unable to load Wi-Fi summary:",
                error
            );

            setStatus(false);

            elements.networkCount.textContent =
                "—";

            elements.openCount.textContent =
                "—";

            elements.hiddenCount.textContent =
                "—";

            elements.strongestRssi.textContent =
                "—";

            elements.weakestRssi.textContent =
                "—";
        }

        try {
            const networks =
                await CyberSwissKnifeAPI.getWifiNetworks();

            updateLatestNetwork(networks);
        } catch (error) {
            console.warn(
                "Unable to load Wi-Fi networks:",
                error
            );

            updateLatestNetwork(null);
        }
    }

    loadDashboard();
})();