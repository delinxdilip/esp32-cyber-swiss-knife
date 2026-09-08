(() => {
    "use strict";


    const elements = {
        statusDot:
            document.getElementById("analyzer-status-dot"),

        statusText:
            document.getElementById("analyzer-status-text"),

        networkCount:
            document.getElementById("analyzer-network-count"),

        openCount:
            document.getElementById("analyzer-open-count"),

        hiddenCount:
            document.getElementById("analyzer-hidden-count"),

        strongest:
            document.getElementById("analyzer-strongest"),

        weakest:
            document.getElementById("analyzer-weakest"),

        signalDistribution:
            document.getElementById("signal-distribution"),

        channelDistribution:
            document.getElementById("channel-distribution"),

        environmentSummary:
            document.getElementById("environment-summary"),

        dominantSignal:
            document.getElementById("dominant-signal"),

        strongNetworkCount:
            document.getElementById("strong-network-count"),

        weakNetworkCount:
            document.getElementById("weak-network-count")
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


    function formatRssi(value) {
        if (
            value === undefined ||
            value === null
        ) {
            return "—";
        }

        return `${value} dBm`;
    }


    function getSignalBucket(rssi) {
        if (rssi >= -50) {
            return "Excellent";
        }

        if (rssi >= -60) {
            return "Good";
        }

        if (rssi >= -70) {
            return "Fair";
        }

        if (rssi >= -80) {
            return "Weak";
        }

        return "Very Weak";
    }


    function updateOverview(data) {
        elements.networkCount.textContent =
            data.network_count ?? "—";

        elements.openCount.textContent =
            data.open_network_count ?? "—";

        elements.hiddenCount.textContent =
            data.hidden_network_count ?? "—";

        elements.strongest.textContent =
            formatRssi(data.strongest_rssi);

        elements.weakest.textContent =
            formatRssi(data.weakest_rssi);
    }


    function clearSignalDistribution() {
        elements.signalDistribution.innerHTML = "";
    }


    function createSignalBar(label, count, total) {
        const row =
            document.createElement("div");

        row.className = "signal-distribution-row";


        const header =
            document.createElement("div");

        header.className = "signal-distribution-header";


        const labelElement =
            document.createElement("span");

        labelElement.textContent = label;


        const countElement =
            document.createElement("strong");

        countElement.textContent = count;


        header.appendChild(labelElement);
        header.appendChild(countElement);


        const track =
            document.createElement("div");

        track.className = "signal-track";


        const fill =
            document.createElement("div");

        fill.className = "signal-fill";


        const percentage =
            total > 0
                ? (count / total) * 100
                : 0;

        fill.style.width =
            `${percentage}%`;


        track.appendChild(fill);

        row.appendChild(header);
        row.appendChild(track);

        return row;
    }


    function renderSignalDistribution(networks) {
        clearSignalDistribution();

        if (
            !Array.isArray(networks) ||
            networks.length === 0
        ) {
            elements.signalDistribution.innerHTML =
                '<div class="analyzer-empty">' +
                'No scan data available.' +
                '</div>';

            return;
        }


        const buckets = {
            Excellent: 0,
            Good: 0,
            Fair: 0,
            Weak: 0,
            "Very Weak": 0
        };


        networks.forEach((network) => {
            const rssi =
                Number(network.rssi);

            if (!Number.isFinite(rssi)) {
                return;
            }

            buckets[getSignalBucket(rssi)] += 1;
        });


        const total =
            networks.length;


        Object.entries(buckets).forEach(
            ([label, count]) => {
                elements.signalDistribution.appendChild(
                    createSignalBar(
                        label,
                        count,
                        total
                    )
                );
            }
        );
    }


    function renderChannelDistribution() {
        /*
         * The current /api/wifi response does not expose
         * channel_counts.
         *
         * Do not calculate or display channel analysis here
         * until the firmware/API exposes that analyzer data.
         */
        elements.channelDistribution.innerHTML =
            '<div class="analyzer-empty">' +
            'Channel distribution will be available ' +
            'when exposed by the API.' +
            '</div>';
    }


    function updateInsights(networks) {
        if (
            !Array.isArray(networks) ||
            networks.length === 0
        ) {
            elements.environmentSummary.textContent =
                "No wireless networks were found.";

            elements.dominantSignal.textContent =
                "—";

            elements.strongNetworkCount.textContent =
                "—";

            elements.weakNetworkCount.textContent =
                "—";

            return;
        }


        const signalBuckets = {
            Excellent: 0,
            Good: 0,
            Fair: 0,
            Weak: 0,
            "Very Weak": 0
        };


        networks.forEach((network) => {
            const rssi =
                Number(network.rssi);

            if (!Number.isFinite(rssi)) {
                return;
            }

            signalBuckets[getSignalBucket(rssi)] += 1;
        });


        let dominant =
            "Very Weak";

        Object.entries(signalBuckets).forEach(
            ([label, count]) => {
                if (
                    count >
                    signalBuckets[dominant]
                ) {
                    dominant = label;
                }
            }
        );


        const strongCount =
            signalBuckets.Excellent +
            signalBuckets.Good;


        const weakCount =
            signalBuckets.Weak +
            signalBuckets["Very Weak"];


        elements.dominantSignal.textContent =
            dominant;

        elements.strongNetworkCount.textContent =
            strongCount;

        elements.weakNetworkCount.textContent =
            weakCount;


        elements.environmentSummary.textContent =
            `${networks.length} wireless networks ` +
            `were discovered. ${strongCount} have ` +
            `strong or good signal levels, while ` +
            `${weakCount} have weak or very weak signals.`;
    }


    async function loadAnalyzer() {
        try {
            const summary =
                await CyberSwissKnifeAPI.getWifi();

            const networkData =
                await CyberSwissKnifeAPI.getWifiNetworks();

            setDeviceStatus(true);

            updateOverview(summary);

            renderSignalDistribution(
                networkData.networks
            );

            renderChannelDistribution();

            updateInsights(
                networkData.networks
            );

        } catch (error) {
            console.error(
                "Unable to load Wi-Fi analyzer data:",
                error
            );

            setDeviceStatus(false);

            elements.environmentSummary.textContent =
                "Unable to load analyzer data.";
        }
    }


    loadAnalyzer();
})();