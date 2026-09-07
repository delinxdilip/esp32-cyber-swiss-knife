const Dashboard={
    async init(){
        try{
            const data=await CyberSwissKnifeAPI.getWifi();
            DashboardMetrics.setStatus(true);
            DashboardMetrics.updateWifiSummary(data);
        }catch(error){
            console.warn("CyberSwissKnife device unavailable:",error);
            DashboardMetrics.setStatus(false);
        }
    }
};
window.Dashboard=Dashboard;
