# esp32-cyber-swiss-knife
🛡️ A portable ESP32-S3 cybersecurity Swiss Army knife for wireless security research, analysis, and experimentation.




CyberSwissKnife/
│
├── firmware/
│   ├── main/
│   │   │
│   │   ├── core/
│   │   │   ├── system/
│   │   │   ├── config/
│   │   │   ├── storage/
│   │   │   ├── logging/
│   │   │   │   ├── system/
│   │   │   │   └── security/
│   │   │   │
│   │   │   └── monitoring/
│   │   │       ├── metrics/
│   │   │       ├── memory/
│   │   │       ├── temperature/
│   │   │       ├── wifi/
│   │   │       ├── bluetooth/
│   │   │       ├── uptime/
│   │   │       └── tasks/
│   │   │
│   │   ├── hardware/
│   │   │
│   │   ├── network/
│   │   │   ├── ap/
│   │   │   │   ├── ap_manager/
│   │   │   │   ├── credentials/
│   │   │   │   └── captive_portal/
│   │   │   │
│   │   │   ├── station/
│   │   │   ├── webserver/
│   │   │   └── websocket/
│   │   │
│   │   ├── wifi/
│   │   │   ├── scanner/
│   │   │   ├── analyzer/
│   │   │   └── lab/
│   │   │
│   │   ├── bluetooth/
│   │   │   ├── scanner/
│   │   │   ├── advertisement/
│   │   │   ├── analyzer/
│   │   │   └── lab/
│   │   │
│   │   └── security/
│   │       ├── wifi/
│   │       ├── bluetooth/
│   │       └── common/
│   │
│   ├── partitions.csv
│   ├── CMakeLists.txt
│   └── sdkconfig.defaults
│
│
├── web/
│   │
│   ├── pages/
│   │   ├── index.html
│   │   ├── metrics.html
│   │   ├── logs.html
│   │   ├── tools.html
│   │   ├── wifi.html
│   │   ├── bluetooth.html
│   │   ├── security.html
│   │   ├── settings.html
│   │   └── about.html
│   │
│   ├── css/
│   │   ├── base.css
│   │   ├── layout.css
│   │   ├── components.css
│   │   ├── dashboard.css
│   │   ├── tools.css
│   │   ├── logs.css
│   │   └── themes.css
│   │
│   ├── js/
│   │   ├── app.js
│   │   ├── router.js
│   │   ├── api.js
│   │   ├── websocket.js
│   │   │
│   │   ├── auth/
│   │   │   ├── auth.js
│   │   │   └── session.js
│   │   │
│   │   ├── dashboard/
│   │   │   ├── dashboard.js
│   │   │   ├── metrics.js
│   │   │   ├── charts.js
│   │   │   └── widgets.js
│   │   │
│   │   ├── wifi/
│   │   │   ├── wifi.js
│   │   │   ├── scanner.js
│   │   │   ├── analyzer.js
│   │   │   └── lab.js
│   │   │
│   │   ├── bluetooth/
│   │   │   ├── bluetooth.js
│   │   │   ├── scanner.js
│   │   │   ├── advertisement.js
│   │   │   ├── analyzer.js
│   │   │   └── lab.js
│   │   │
│   │   ├── security/
│   │   │   ├── security.js
│   │   │   ├── wifi.js
│   │   │   ├── bluetooth.js
│   │   │   └── lab.js
│   │   │
│   │   ├── logs/
│   │   │   ├── logs.js
│   │   │   ├── system-logs.js
│   │   │   └── security-logs.js
│   │   │
│   │   ├── settings/
│   │   │   ├── settings.js
│   │   │   ├── ap-settings.js
│   │   │   ├── network-settings.js
│   │   │   └── storage-settings.js
│   │   │
│   │   └── components/
│   │       ├── navigation.js
│   │       ├── modal.js
│   │       ├── toast.js
│   │       ├── cards.js
│   │       └── loading.js
│   │
│   └── assets/
│       ├── icons/
│       ├── images/
│       └── fonts/
│
│
├── storage/
│   └── README.md
│
├── docs/
│   ├── architecture.md
│   ├── api.md
│   ├── storage.md
│   ├── security.md
│   └── development.md
│
├── tools/
│
├── .env.example
├── .gitignore
├── LICENSE
└── README.md