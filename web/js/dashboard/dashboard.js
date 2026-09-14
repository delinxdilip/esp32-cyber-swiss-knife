const Dashboard = {
    pollInterval: null,

    async refresh() {
        try {
            const state =
                await CyberSwissKnifeAPI.getState();

            DashboardMetrics.setStatus(true);
            DashboardMetrics.updateState(state);
        } catch (error) {
            console.warn(
                "CyberSwissKnife device unavailable:",
                error);

            DashboardMetrics.setStatus(false);
        }
    },

    async init() {
        await this.refresh();

        if (this.pollInterval) {
            clearInterval(this.pollInterval);
        }

        this.pollInterval =
            setInterval(() => {
                this.refresh();
            }, 5000);
    }
};

window.Dashboard = Dashboard;