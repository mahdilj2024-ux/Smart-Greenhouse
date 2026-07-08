/**
 * Smart Greenhouse - Real MQTT/WebSockets Control & Monitoring System
 * Powered By MLJ
 */

document.addEventListener("DOMContentLoaded", () => {
    // === تنظیمات اتصال به رزبری پای ===
    const RASPBERRY_PI_IP = "10.95.51.100"; // آی‌پی صحیح رزبری پای شما ست شد
    const MQTT_SUB_TOPIC = "greenhouse/+/status";
    const MQTT_PUB_LAMP = "greenhouse/main/lamp/set";

    // === 1. DOM ELEMENTS SELECTION ===
    const totalCurrentInp = document.getElementById("totalCurrent");
    const liveTimeEl = document.getElementById("liveTime");
    const liveDateEl = document.getElementById("liveDate");
    const footerYearEl = document.getElementById("footerYear");

    const mqttDot = document.getElementById("mqttDot");
    const mqttText = document.getElementById("mqttText");

    const mainTempInp = document.getElementById("maintemp");
    const mainHemuInp = document.getElementById("mainhemu");
    const mainPressInp = document.getElementById("barometer");
    const fanInp = document.getElementById("fan"); // نام صحیح المان اینجاست
    const lampBtn = document.getElementById("lampBtn");

    const soilHemu1Inp = document.getElementById("soilhemu1");
    const pumpStatus1Inp = document.getElementById("pumpstatus1");

    const lineSelect = document.getElementById("lineSelect");
    const metricSelect = document.getElementById("metricSelect");
    const dateFilter = document.getElementById("dateFilter");
    const customDateInp = document.getElementById("customDate");
    const applyFilterBtn = document.getElementById("applyFilterBtn");
    const filterHint = document.getElementById("filterHint");

    const chartCards = document.querySelectorAll(".chart-card");

    // === 2. INITIALIZATION & CLOCK ===
    if (footerYearEl) footerYearEl.textContent = new Date().getFullYear();

    function updateClock() {
        const now = new Date();
        if (liveTimeEl) liveTimeEl.textContent = now.toTimeString().split(' ')[0];
        if (liveDateEl) {
            const year = now.getFullYear();
            const month = String(now.getMonth() + 1).padStart(2, '0');
            const day = String(now.getDate()).padStart(2, '0');
            liveDateEl.textContent = `${year}/${month}/${day}`;
        }
    }
    setInterval(updateClock, 1000);
    updateClock();

    // === 3. REAL MQTT CONNECTION OVER WEBSOCKETS ===
    const mqttUrl = `ws://${RASPBERRY_PI_IP}:9001`;
    const client = mqtt.connect(mqttUrl);

    client.on("connect", () => {
        console.log("Connected to Mosquitto Broker on Raspberry Pi!");
        if (mqttDot && mqttText) {
            mqttDot.style.backgroundColor = "#10b981";
            mqttDot.style.boxShadow = "0 0 8px #10b981";
            mqttText.textContent = "Connected";
        }
        client.subscribe(MQTT_SUB_TOPIC, (err) => {
            if (!err) console.log(`Subscribed to topic: ${MQTT_SUB_TOPIC}`);
        });
        // همچنین برای هماهنگ شدن دکمه لامپ، در تاپیک وضعیت لامپ هم سابسکرایب می‌کنیم
        client.subscribe("greenhouse/main/lamp");
    });

    client.on("error", (err) => {
        console.error("MQTT Connection Error: ", err);
        if (mqttDot && mqttText) {
            mqttDot.style.backgroundColor = "#ef4444";
            mqttDot.style.boxShadow = "0 0 8px #ef4444";
            mqttText.textContent = "Error";
        }
    });

    client.on("close", () => {
        if (mqttDot && mqttText) {
            mqttDot.style.backgroundColor = "#f59e0b";
            mqttDot.style.boxShadow = "0 0 8px #f59e0b";
            mqttText.textContent = "Reconnecting...";
        }
    });

    // === 4. RECEIVING LIVE DATA FROM RASPBERRY PI ===
    client.on("message", (topic, message) => {
        try {
            const data = JSON.parse(message.toString());

            // ۱. پردازش اطلاعات سنسورها (فقط از برد اصلی)
            if (topic === "greenhouse/main/status") {
                if (data.temp !== undefined && mainTempInp) mainTempInp.value = data.temp;
                if (data.hemu !== undefined && mainHemuInp) mainHemuInp.value = data.hemu;
                if (data.press !== undefined && mainPressInp) mainPressInp.value = data.press;
                if (data.fan !== undefined && fanInp) fanInp.value = data.fan; // غلط املایی به fanInp فیکس شد
                if (data.current !== undefined && totalCurrentInp) totalCurrentInp.value = data.current;
            }

            // ۲. پردازش وضعیت لامپ 
            if (topic === "greenhouse/main/lamp") {
                if (data.lamp !== undefined && lampBtn) {
                    lampBtn.setAttribute("data-state", data.lamp.toLowerCase());
                    lampBtn.textContent = `Lamp of the greenhouse: ${data.lamp}`;

                    if (data.lamp === "ON") {
                        lampBtn.className = "btn btn-success"; // فرض بر وجود استایل دکمه روشن
                        lampBtn.style.backgroundColor = "#10b981";
                    } else {
                        lampBtn.className = "btn btn-danger";
                        lampBtn.style.backgroundColor = "#ef4444";
                    }
                }
            }
        } catch (e) {
            console.error("Error parsing MQTT message:", e);
        }
    });

    // === 5. INTERACTIVE ACTUATORS (Publishing to Lamp) ===
    if (lampBtn) {
        lampBtn.addEventListener("click", () => {
            const currentState = lampBtn.getAttribute("data-state");
            let newState = "OFF";

            if (currentState === "off" || currentState === "OFF") {
                newState = "ON";
            } else {
                newState = "OFF";
            }

            const payload = JSON.stringify({ state: newState });
            client.publish(MQTT_PUB_LAMP, payload, { qos: 1 });
            console.log(`Published to ${MQTT_PUB_LAMP}: ${payload}`);
        });
    }

    // === 6. DYNAMIC CHART FILTERS ===
    const metricsConfig = {
        main: [
            { value: "temp", text: "Average Temperature" },
            { value: "hemu", text: "Average Humidity" },
            { value: "press", text: "Barometric Pressure" }
        ],
        "1": [
            { value: "soil", text: "Soil Humidity" },
            { value: "pump", text: "Pump Active Duration" }
        ],
        "2": [
            { value: "soil", text: "Soil Humidity" }
        ],
        "3": [
            { value: "soil", text: "Soil Humidity" }
        ]
    };

    function populateMetrics() {
        const selectedLine = lineSelect.value;
        const availableMetrics = metricsConfig[selectedLine] || [];

        if (metricSelect) {
            metricSelect.innerHTML = "";
            availableMetrics.forEach(metric => {
                const opt = document.createElement("option");
                opt.value = metric.value;
                opt.textContent = metric.text;
                metricSelect.appendChild(opt);
            });
        }
    }

    if (lineSelect) {
        lineSelect.addEventListener("change", populateMetrics);
        populateMetrics();
    }

    if (dateFilter) {
        dateFilter.addEventListener("change", () => {
            if (dateFilter.value === "custom") {
                customDateInp.style.display = "inline-block";
            } else {
                customDateInp.style.display = "none";
            }
        });
    }

    if (applyFilterBtn) {
        applyFilterBtn.addEventListener("click", () => {
            const line = lineSelect.value;
            const metric = metricSelect ? metricSelect.value : "temp";
            const range = dateFilter.value;
            const customDate = customDateInp.value;

            let details = `Line: ${line.toUpperCase()} | Metric: ${metric.toUpperCase()} | Range: ${range}`;
            if (range === "custom" && customDate) details += ` (${customDate})`;

            if (filterHint) filterHint.textContent = `Applied Filter -> ${details}`;

            Object.keys(renderedCharts).forEach(key => {
                if (renderedCharts[key]) {
                    updateChartData(key);
                }
            });
        });
    }

    // === 7. LAZY-LOADING CHARTS (Chart.js Integration) ===
    const renderedCharts = { line: null, bar: null, pie: null };

    chartCards.forEach(card => {
        card.addEventListener("click", function() {
            const chartKey = this.getAttribute("data-chart-key");
            const placeholder = this.querySelector(`[data-placeholder="${chartKey}"]`);
            const wrap = this.querySelector(`[data-wrap="${chartKey}"]`);

            if (placeholder && placeholder.style.display === "none") return;

            if (placeholder) placeholder.style.display = "none";
            if (wrap) wrap.style.display = "block";

            initChart(chartKey);
        });
    });

    function initChart(type) {
        const ctx = document.getElementById(`${type}Chart`);
        if (!ctx || renderedCharts[type]) return;

        const mockLabels = ["00:00", "04:00", "08:00", "12:00", "16:00", "20:00"];
        let chartConfig = {};

        if (type === "line") {
            chartConfig = {
                type: 'line',
                data: {
                    labels: mockLabels,
                    datasets: [{
                        label: 'Metric Value Trend',
                        data: [0, 0, 0, 0, 0, 0],
                        borderColor: '#3b82f6',
                        backgroundColor: 'rgba(59, 130, 246, 0.1)',
                        tension: 0.3,
                        fill: true
                    }]
                },
                options: { responsive: true, maintainAspectRatio: false }
            };
        } else if (type === "bar") {
            chartConfig = {
                type: 'bar',
                data: {
                    labels: ['Line 1', 'Line 2', 'Line 3'],
                    datasets: [{
                        label: 'Comparative Level',
                        data: [0, 0, 0],
                        backgroundColor: ['#10b981', '#f59e0b', '#ef4444']
                    }]
                },
                options: { responsive: true, maintainAspectRatio: false }
            };
        } else if (type === "pie") {
            chartConfig = {
                type: 'pie',
                data: {
                    labels: ['Optimal', 'Warning', 'Critical'],
                    datasets: [{
                        data: [100, 0, 0],
                        backgroundColor: ['#10b981', '#f59e0b', '#ef4444']
                    }]
                },
                options: { responsive: true, maintainAspectRatio: false }
            };
        }

        renderedCharts[type] = new Chart(ctx, chartConfig);
        updateChartData(type);
    }

    function updateChartData(type) {
        const chart = renderedCharts[type];
        if (!chart) return;

        const line = lineSelect.value;
        const metric = metricSelect ? metricSelect.value : "temp";
        const range = dateFilter.value;
        const customDate = customDateInp.value;

        let url = `get_chart_data.php?line=${line}&metric=${metric}&range=${range}`;
        if (range === "custom" && customDate) {
            url += `&custom_date=${customDate}`;
        }

        fetch(url)
            .then(response => response.json())
            .then(data => {
                if (data.error) {
                    console.error("Database error:", data.error);
                    return;
                }

                const labels = data.map(item => item.time_label);
                const values = data.map(item => item.value);

                if (type === 'line' || type === 'bar') {
                    chart.data.labels = labels;
                    chart.data.datasets[0].data = values;
                    chart.data.datasets[0].label = `${metric.toUpperCase()} (${line.toUpperCase()})`;
                } else if (type === 'pie') {
                    chart.data.datasets[0].data = values.slice(0, 3);
                }

                chart.update();
            })
            .catch(err => console.error("Fetch error:", err));
    }
});