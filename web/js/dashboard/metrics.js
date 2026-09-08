const DashboardMetrics={
    setStatus(online){
        const dot=document.getElementById("status-dot");
        const text=document.getElementById("status-text");
        const detail=document.getElementById("connection-detail");
        if(!dot||!text)return;
        dot.classList.toggle("online",online);
        dot.classList.toggle("offline",!online);
        text.textContent=online?"ONLINE":"OFFLINE";
        if(detail)detail.textContent=online?"ESP32-S3 connected":"ESP32-S3 unavailable";
    },
    updateWifiSummary(data){
        if(!data)return;
        window.cyberSwissKnifeState={...(window.cyberSwissKnifeState||{}),wifi:data};
    }
};
window.DashboardMetrics=DashboardMetrics;
