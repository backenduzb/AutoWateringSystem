<script lang="ts">
    import { onMount } from "svelte";
    import Chart from "chart.js/auto";
    import { ROOT_URL } from "$lib";
    
    let current: any = $state(null);
    let history: any[] = $state([]);
    let isLoading = $state(true);
    let wsStatus = $state('connecting'); 
    let lastUpdate = $state<Date | null>(null);
    let chartCanvas: HTMLCanvasElement;
    let chart: Chart | null = null;
    
    let motorState = $state(false);
    let motorLoading = $state(false);
    
    let ws: WebSocket | null = null;
    let motorWs: WebSocket | null = null; 
    let reconnectTimer: any = null;
    let reconnectAttempts = 0;
    
    function getStatusClass() {
        if (wsStatus === 'connected') return 'bg-teal-400 animate-pulse';
        if (wsStatus === 'connecting') return 'bg-yellow-500 animate-pulse';
        return 'bg-red-500';
    }

    function getStatusText() {
        if (wsStatus === 'connected') return 'Live';
        if (wsStatus === 'connecting') return 'Connecting...';
        return 'Reconnecting...';
    }

    function getStatusTextValue(value: number) {
        if (value == null) return '--';
        if (value < 30) return 'Quruq 🌵';
        if (value < 60) return 'Normal 🌱';
        return 'Nam 💧';
    }

    function getStatusColor(value: number) {
        if (value == null) return 'text-gray-400';
        if (value < 30) return 'text-yellow-400';
        if (value < 60) return 'text-teal-400';
        return 'text-cyan-400';
    }

    function getProgressColor(value: number) {
        if (value == null) return 'bg-gray-500';
        if (value < 30) return 'bg-yellow-500';
        if (value < 60) return 'bg-teal-500';
        return 'bg-cyan-500';
    }
    
    async function controlMotor(action: 'on' | 'off') {
        const token = localStorage.getItem("access");
        
        if (!token) {
            return;
        }
        
        motorLoading = true;
        
        try {
            const response = await fetch(`${ROOT_URL}/sensors/motor/control/`, {
                method: 'POST',
                headers: {
                    'Content-Type': 'application/json',
                    'Authorization': `Token ${token}`
                },
                body: JSON.stringify({ state: action === 'on' })
            });
            
            if (!response.ok) {
                throw new Error(`HTTP ${response.status}`);
            }
            
            const result = await response.json();
            motorState = action === 'on';
            
        } catch (err) {
            
            if (motorWs && motorWs.readyState === WebSocket.OPEN) {
                motorWs.send(JSON.stringify({
                    command: action === 'on' ? 'ON' : 'OFF'
                }));
                motorState = action === 'on';
            }
        } finally {
            motorLoading = false;
        }
    }

    async function loadLatest() {
        const token = localStorage.getItem("access");
        
        if (!token) {
            isLoading = false;
            return;
        }
    
        try {
            const res = await fetch(`${ROOT_URL}/sensors/latest/`, {
                headers: {
                    Authorization: `Token ${token}`
                }
            });
    
            if (!res.ok) throw new Error(`HTTP ${res.status}`);
            
            const json = await res.json();
            current = json.data;
            lastUpdate = new Date();
            
            if (current && current.value != null) {
                history = [{
                    value: current.value,
                    timestamp: current.timestamp || new Date().toISOString()
                }];
            }
            
            if (json.motor_state !== undefined) {
                motorState = json.motor_state;
            }
            
        } catch (err) {
        } finally {
            isLoading = false;
        }
    }
    
    function updateChartData() {
        if (!chart) return;
        
        if (history.length === 0) {
            chart.data.labels = [];
            chart.data.datasets[0].data = [];
            chart.update('none');
            return;
        }
        
        const labels = history.map(h => {
            const date = new Date(h.timestamp);
            return date.toLocaleTimeString('uz-UZ', { 
                hour: '2-digit', 
                minute: '2-digit',
                second: '2-digit'
            });
        });
        
        const values = history.map(h => h.value);
        
        chart.data.labels = labels;
        chart.data.datasets[0].data = values;
        chart.update('active');
        
    }
    
    function addNewReading(newData: any) {
        if (!newData || newData.value == null) return;
        
        
        current = newData;
        lastUpdate = new Date();
        
        const newPoint = {
            value: newData.value,
            timestamp: newData.timestamp || new Date().toISOString()
        };
        
        const updatedHistory = [...history, newPoint];
        
        if (updatedHistory.length > 20) {
            history = updatedHistory.slice(-20);
        } else {
            history = updatedHistory;
        }
        
        if (chart) {
            updateChartData();
        }
        
    }
    
    function initChart() {
        if (!chartCanvas) {
            setTimeout(initChart, 100);
            return;
        }
        
        const ctx = chartCanvas.getContext('2d');
        if (!ctx) return;
        
        if (chart) {
            chart.destroy();
        }
        
        chart = new Chart(ctx, {
            type: "line",
            data: {
                labels: [],
                datasets: [
                    {
                        label: "Namlik (%)",
                        data: [],
                        borderColor: "#14b8a6",
                        backgroundColor: "rgba(20, 184, 166, 0.1)",
                        borderWidth: 2,
                        pointRadius: 3,
                        pointBackgroundColor: "#14b8a6",
                        pointBorderColor: "#0f172a",
                        pointBorderWidth: 2,
                        pointHoverRadius: 6,
                        tension: 0.4,
                        fill: true
                    }
                ]
            },
            options: {
                responsive: true,
                maintainAspectRatio: false,
                animation: {
                    duration: 750,
                    easing: 'easeInOutQuart'
                },
                plugins: {
                    legend: {
                        display: true,
                        position: "top",
                        labels: {
                            color: "#94a3b8",
                            font: { size: 12, weight: "500" },
                            boxWidth: 12,
                            boxHeight: 12
                        }
                    },
                    tooltip: {
                        mode: "index",
                        intersect: false,
                        backgroundColor: "#1e293b",
                        titleColor: "#f1f5f9",
                        bodyColor: "#94a3b8",
                        borderColor: "#14b8a6",
                        borderWidth: 1,
                        callbacks: {
                            label: (context) => `Namlik: ${context.parsed.y}%`
                        }
                    }
                },
                scales: {
                    y: {
                        beginAtZero: true,
                        max: 100,
                        grid: { color: "#334155", drawBorder: false },
                        title: { 
                            display: true, 
                            text: "Namlik (%)", 
                            color: "#94a3b8", 
                            font: { size: 11 } 
                        },
                        ticks: { 
                            color: "#94a3b8", 
                            stepSize: 20, 
                            callback: (v) => v + "%" 
                        }
                    },
                    x: {
                        grid: { display: false, drawBorder: false },
                        title: { 
                            display: true, 
                            text: "Vaqt", 
                            color: "#94a3b8", 
                            font: { size: 11 } 
                        },
                        ticks: { 
                            color: "#94a3b8", 
                            maxRotation: 45, 
                            minRotation: 45,
                            autoSkip: true,
                            maxTicksLimit: 10
                        }
                    }
                },
                interaction: { 
                    mode: 'nearest', 
                    axis: 'x', 
                    intersect: false 
                }
            }
        });
        
        if (history.length > 0) {
            updateChartData();
        }
        
    }
    
    function connectWebSocket() {
        const token = localStorage.getItem("access");
        if (!token) {
            wsStatus = 'error';
            setTimeout(connectWebSocket, 5000);
            return;
        }
    
        wsStatus = 'connecting';
        reconnectAttempts++;
        
        const wsUrl = ROOT_URL.replace("https://", "wss://").replace("http://", "ws://") + `/ws/updates/?token=${token}`;
    
        try {
            ws = new WebSocket(wsUrl);
            
            ws.onopen = () => {
                wsStatus = 'connected';
                reconnectAttempts = 0;
            };
        
            ws.onmessage = (e) => {
                try {
                    const msg = JSON.parse(e.data);
        
                    let newData = null;
                    
                    if (msg.type === "reading" && msg.data) {
                        newData = msg.data;
                    } else if (msg.type === "sensor_update" && msg.data) {
                        newData = msg.data;
                    } else if (msg.value != null) {
                        newData = msg;
                    }
                    
                    if (newData && newData.value != null) {
                        addNewReading(newData);
                    }
                    
                    if (msg.motor_state !== undefined) {
                        motorState = msg.motor_state;
                    }
                    
                } catch (err) {
                }
            };
        
            ws.onerror = (err) => {
                wsStatus = 'error';
            };
        
            ws.onclose = (event) => {
                wsStatus = 'error';
                
                if (reconnectTimer) clearTimeout(reconnectTimer);
                const delay = Math.min(30000, 1000 * Math.pow(2, Math.min(reconnectAttempts, 5)));
                
                reconnectTimer = setTimeout(() => {
                    connectWebSocket();
                }, delay);
            };
        } catch (err) {
            wsStatus = 'error';
            setTimeout(connectWebSocket, 5000);
        }
    }
    
    function connectMotorWebSocket() {
        const token = localStorage.getItem("access");
        if (!token) {
            setTimeout(connectMotorWebSocket, 5000);
            return;
        }
        
        const wsUrl = ROOT_URL.replace("https://", "wss://").replace("http://", "ws://") + `/ws/motor/control/?token=${token}`;
        
        try {
            motorWs = new WebSocket(wsUrl);
            
            motorWs.onopen = () => {
            };
            
            motorWs.onerror = (err) => {
            };
            
            motorWs.onclose = () => {
                setTimeout(connectMotorWebSocket, 5000);
            };
            
        } catch (err) {
            setTimeout(connectMotorWebSocket, 5000);
        }
    }

    function disconnectWebSockets() {
        if (reconnectTimer) {
            clearTimeout(reconnectTimer);
            reconnectTimer = null;
        }
        if (ws) {
            ws.close();
            ws = null;
        }
        if (motorWs) {
            motorWs.close();
            motorWs = null;
        }
        reconnectAttempts = 0;
    }
    
    $effect(() => {
        if (history.length > 0 && chart) {
            updateChartData();
        }
    });
    
    $effect(() => {
        if (chartCanvas && !chart) {
            setTimeout(() => {
                initChart();
            }, 50);
        }
    });
    
    onMount(async () => {
        
        await loadLatest();
        connectWebSocket();
        connectMotorWebSocket();
        
        return () => {
            disconnectWebSockets();
            if (chart) {
                chart.destroy();
                chart = null;
            }
        };
    });
</script>

<div class="min-h-screen bg-black">
    <div class="fixed p-6 space-y-6 mt-10 w-[80%] right-10 top-6 h-screen overflow-y-auto">
        <div class="flex items-center justify-between">
            <div>
                <h1 class="text-2xl font-bold text-white">Sensor Dashboard</h1>
                <p class="text-sm text-teal-500 mt-1">Real-time monitoring</p>
            </div>

            <div class="flex items-center gap-3">
                {#if lastUpdate}
                    <span class="text-xs text-teal-500">
                        Yangilangan: {new Date(lastUpdate).toLocaleTimeString()}
                    </span>
                {/if}
                
                <div class="flex items-center gap-2">
                    <div class="w-2 h-2 rounded-full {getStatusClass()}"></div>
                    <span class="text-sm text-slate-400">{getStatusText()}</span>
                </div>
            </div>
        </div>
        <div class="grid grid-cols-3 gap-5">
            <div class="group relative overflow-hidden rounded-2xl bg-transparent backdrop-blur-md 
                        border border-teal-700 hover:border-teal-500/30 transition-all duration-300">
                <div class="absolute top-0 right-0 w-32 h-32 bg-teal-500/5 rounded-full blur-2xl -mr-16 -mt-16"></div>
                <div class="relative p-5">
                    <div class="flex items-center justify-between mb-3">
                        <div class="p-2 rounded-xl bg-teal-500/10">
                            <svg class="w-5 h-5 text-teal-400" fill="none" stroke="currentColor" viewBox="0 0 24 24">
                                <path stroke-linecap="round" stroke-linejoin="round" stroke-width="2" 
                                      d="M5 3v4M3 5h4M6 17v4m-2-2h4m5-16l2.286 6.857L21 12l-5.714 2.143L13 21l-2.286-6.857L5 12l5.714-2.143L13 3z"/>
                            </svg>
                        </div>
                        <span class="text-xs text-teal-500">SO'NGI MA'LUMOT</span>
                    </div>
                    
                    {#if isLoading}
                        <div class="h-12 w-24 bg-slate-700/50 rounded-lg animate-pulse"></div>
                    {:else}
                        <h1 class="text-4xl font-bold bg-gradient-to-r from-teal-400 to-cyan-400 bg-clip-text text-transparent">
                            {current?.value ?? '--'}%
                        </h1>
                    {/if}
                    
                    <p class="text-sm text-teal-500 mt-2">
                        {#if current?.value != null}
                            <span class={getStatusColor(current.value)}>
                                {getStatusTextValue(current.value)}
                            </span>
                        {/if}
                    </p>
                </div>
            </div>
            
            <div class="group relative overflow-hidden rounded-2xl bg-transparent backdrop-blur-md 
                        border border-teal-700 hover:border-teal-500/30 transition-all duration-300">
                <div class="absolute top-0 right-0 w-32 h-32 bg-cyan-500/5 rounded-full blur-2xl -mr-16 -mt-16"></div>
                <div class="relative p-5">
                    <div class="flex items-center justify-between mb-3">
                        <div class="p-2 rounded-xl bg-cyan-500/10">
                            <svg class="w-5 h-5 text-cyan-400" fill="none" stroke="currentColor" viewBox="0 0 24 24">
                                <path stroke-linecap="round" stroke-linejoin="round" stroke-width="2" 
                                      d="M9 12l2 2 4-4m6 2a9 9 0 11-18 0 9 9 0 0118 0z"/>
                            </svg>
                        </div>
                        <span class="text-xs text-teal-500">TIZIM HOLATI</span>
                    </div>
                    
                    <h1 class="text-2xl font-semibold {getStatusColor(current?.value)}">
                        {#if current?.value == null}
                            --
                        {:else if current.value < 30}
                            Quruq 🌵
                        {:else if current.value < 60}
                            Normal 🌱
                        {:else}
                            Nam 💧
                        {/if}
                    </h1>
                    
                    <div class="mt-3 h-1.5 w-full bg-slate-700 rounded-full overflow-hidden">
                        <div 
                            class="h-full transition-all duration-500 rounded-full {getProgressColor(current?.value)}"
                            style="width: {current?.value ?? 0}%"
                        ></div>
                    </div>
                </div>
            </div>
            
            <div class="group relative overflow-hidden rounded-2xl bg-transparent backdrop-blur-md 
                        border border-teal-700 hover:border-teal-500/30 transition-all duration-300">
                <div class="absolute top-0 right-0 w-32 h-32 bg-purple-500/5 rounded-full blur-2xl -mr-16 -mt-16"></div>
                <div class="relative p-5">
                    <div class="flex items-center justify-between mb-3">
                        <div class="p-2 rounded-xl bg-purple-500/10">
                            <svg class="w-5 h-5 text-purple-400" fill="none" stroke="currentColor" viewBox="0 0 24 24">
                                <path stroke-linecap="round" stroke-linejoin="round" stroke-width="2" 
                                      d="M12 19l9 2-9-18-9 18 9-2zm0 0v-8"/>
                            </svg>
                        </div>
                        <span class="text-xs text-teal-500">MOTOR BOSHQARISH</span>
                    </div>
                    
                    <div class="flex gap-3 mt-2">
                        <button 
                            onclick={() => controlMotor('on')}
                            disabled={motorLoading || motorState}
                            class="flex-1 py-3 rounded-xl font-semibold transition-all duration-300
                                   {motorState 
                                     ? 'bg-emerald-500/20 border border-emerald-500 text-emerald-400 cursor-not-allowed' 
                                     : 'bg-gradient-to-r from-emerald-600 to-emerald-700 hover:from-emerald-500 hover:to-emerald-600 text-white shadow-lg hover:shadow-emerald-500/25'
                                   } disabled:opacity-50 disabled:cursor-not-allowed"
                        >
                            {#if motorLoading && !motorState}
                                <svg class="animate-spin h-5 w-5 mx-auto" fill="none" viewBox="0 0 24 24">
                                    <circle class="opacity-25" cx="12" cy="12" r="10" stroke="currentColor" stroke-width="4"></circle>
                                    <path class="opacity-75" fill="currentColor" d="M4 12a8 8 0 018-8V0C5.373 0 0 5.373 0 12h4zm2 5.291A7.962 7.962 0 014 12H0c0 3.042 1.135 5.824 3 7.938l3-2.647z"></path>
                                </svg>
                            {:else}
                                🔛 YOQISH
                            {/if}
                        </button>
                        
                        <button 
                            onclick={() => controlMotor('off')}
                            disabled={motorLoading || !motorState}
                            class="flex-1 py-3 rounded-xl font-semibold transition-all duration-300
                                   {!motorState 
                                     ? 'bg-red-500/20 border border-red-500 text-red-400 cursor-not-allowed' 
                                     : 'bg-gradient-to-r from-red-600 to-red-700 hover:from-red-500 hover:to-red-600 text-white shadow-lg hover:shadow-red-500/25'
                                   } disabled:opacity-50 disabled:cursor-not-allowed"
                        >
                            {#if motorLoading && motorState}
                                <svg class="animate-spin h-5 w-5 mx-auto" fill="none" viewBox="0 0 24 24">
                                    <circle class="opacity-25" cx="12" cy="12" r="10" stroke="currentColor" stroke-width="4"></circle>
                                    <path class="opacity-75" fill="currentColor" d="M4 12a8 8 0 018-8V0C5.373 0 0 5.373 0 12h4zm2 5.291A7.962 7.962 0 014 12H0c0 3.042 1.135 5.824 3 7.938l3-2.647z"></path>
                                </svg>
                            {:else}
                                🔴 O'CHIRISH
                            {/if}
                        </button>
                    </div>
                    
                    <div class="mt-3 flex items-center justify-center gap-2">
                        <div class="w-2 h-2 rounded-full {motorState ? 'bg-emerald-500 animate-pulse' : 'bg-red-500'}"></div>
                        <span class="text-xs text-slate-400">
                            Motor {motorState ? 'yoqilgan ✅' : 'o\'chirilgan ❌'}
                        </span>
                    </div>
                </div>
            </div>
        </div>
        
        <div class="rounded-2xl bg-transparent backdrop-blur-md border border-teal-700 
                    hover:border-teal-500/30 transition-all duration-300 overflow-hidden">
            <div class="border-b border-teal-700 px-6 py-4">
                <div class="flex items-center justify-between">
                    <div>
                        <h2 class="font-semibold text-slate-200">Namlik grafigi</h2>
                        <p class="text-xs text-teal-500 mt-0.5">So'ngi 20 ta o'lchov</p>
                    </div>
                    <div class="flex items-center gap-2">
                        <div class="flex items-center gap-1.5">
                            <div class="w-2 h-2 rounded-full bg-teal-500"></div>
                            <span class="text-xs text-teal-500">Namlik (%)</span>
                        </div>
                    </div>
                </div>
            </div>
            
            <div class="p-6">
                <div class="h-80 w-full">
                    <canvas bind:this={chartCanvas}></canvas>
                </div>
            </div>
        </div>
    </div>
</div>