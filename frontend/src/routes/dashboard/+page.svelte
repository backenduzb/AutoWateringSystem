<script lang="ts">
    import { onMount } from "svelte";
    import Chart from "chart.js/auto";
    import { fly } from "svelte/transition";
    import { ROOT_URL } from "$lib";
    
    // ============ STATE ============
    let current: any = $state(null);
    let history: any[] = $state([]);
    let isLoading = $state(true);
    let wsStatus = $state('connecting'); 
    let lastUpdate = $state<Date | null>(null);
    let chartCanvas: HTMLCanvasElement;
    let chart: Chart | null = $state(null);
    
    // ============ HELPER FUNCTIONS ============
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

    // ============ CHART UPDATE ============
    function updateChart() {
        if (!chart) {
            console.log("⚠️ Chart not initialized");
            return;
        }
        
        if (history.length === 0) {
            console.log("📊 No history data");
            return;
        }

        console.log(`📊 Updating chart with ${history.length} points`);
        
        chart.data.labels = history.map(i => {
            const date = new Date(i.timestamp);
            return date.toLocaleTimeString([], { hour: '2-digit', minute: '2-digit', second: '2-digit' });
        });

        chart.data.datasets[0].data = history.map(i => i.value);
        chart.update('none'); // 'none' - animatsiyasiz tezroq yangilanish
    }

    // ============ LOAD LATEST ============
    async function loadLatest() {
        const token = localStorage.getItem("access");
        
        if (!token) {
            console.error("❌ No token found");
            isLoading = false;
            return;
        }

        try {
            const res = await fetch(`${ROOT_URL}/sensors/latest/`, {
                headers: {
                    Authorization: `Bearer ${token}`
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
                updateChart();
            }
        } catch (err) {
            console.error("❌ Failed to load latest:", err);
        } finally {
            isLoading = false;
        }
    }

    // ============ WEBSOCKET ============
    let ws: WebSocket | null = null;
    let reconnectTimer: any = null;

    function connectWebSocket() {
        const token = localStorage.getItem("access");
        if (!token) {
            console.error("❌ No token for WebSocket");
            return;
        }
    
        wsStatus = 'connecting';
        
        const wsUrl = ROOT_URL.replace("https://", "wss://") + `/ws/updates/?token=${token}`;
        console.log("🔌 Connecting to WebSocket:", wsUrl);
    
        ws = new WebSocket(wsUrl);
    
        ws.onopen = () => {
            console.log("✅ WebSocket connected");
            wsStatus = 'connected';
        };
    
        ws.onmessage = (e) => {
            try {
                const msg = JSON.parse(e.data);
                console.log("📨 Received:", msg);
    
                let newData = null;
                
                if (msg.type === "reading" && msg.data) {
                    newData = msg.data;
                } else if (msg.type === "sensor_update" && msg.data) {
                    newData = msg.data;
                } else if (msg.value != null) {
                    newData = msg;
                }
                
                if (newData && newData.value != null) {
                    // MUHIM: Yangi state yaratish (reaktivlik uchun)
                    current = { ...newData };
                    lastUpdate = new Date();
                    
                    // History ni yangilash (reaktivlik uchun yangi array)
                    const newHistoryPoint = {
                        value: newData.value,
                        timestamp: newData.timestamp || new Date().toISOString()
                    };
                    
                    history = [...history.slice(-19), newHistoryPoint];
                    
                    console.log(`📊 History: ${history.length} points, Current: ${newData.value}%`);
                    
                    // Chartni yangilash
                    updateChart();
                }
            } catch (err) {
                console.error("❌ Parse error:", err);
            }
        };
    
        ws.onerror = (err) => {
            console.error("❌ WS error:", err);
            wsStatus = 'error';
        };
    
        ws.onclose = () => {
            console.log("❌ WS closed");
            wsStatus = 'error';
            
            // Auto reconnect
            if (reconnectTimer) clearTimeout(reconnectTimer);
            reconnectTimer = setTimeout(() => {
                console.log("🔄 Reconnecting...");
                connectWebSocket();
            }, 3000);
        };
    }

    function disconnectWebSocket() {
        if (reconnectTimer) clearTimeout(reconnectTimer);
        if (ws) {
            ws.close();
            ws = null;
        }
    }

    // ============ INIT CHART ============
    function initChart() {
        if (!chartCanvas) {
            console.log("⚠️ Canvas not ready");
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
                    duration: 0 // Real-time uchun animatsiyani o'chirish
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
                        title: { display: true, text: "Namlik (%)", color: "#94a3b8", font: { size: 11 } },
                        ticks: { color: "#94a3b8", stepSize: 20, callback: (v) => v + "%" }
                    },
                    x: {
                        grid: { display: false, drawBorder: false },
                        title: { display: true, text: "Vaqt", color: "#94a3b8", font: { size: 11 } },
                        ticks: { color: "#94a3b8", maxRotation: 45, minRotation: 45 }
                    }
                },
                interaction: { mode: 'nearest', axis: 'x', intersect: false }
            }
        });
        
        console.log("✅ Chart initialized");
        
        // Agar history bo'lsa, chartni yangilash
        if (history.length > 0) {
            updateChart();
        }
    }

    // ============ WATCH HISTORY FOR CHART ============
    // Svelte 5 da history o'zgarganda chartni yangilash
    $effect(() => {
        if (history.length > 0 && chart) {
            updateChart();
        }
    });

    // ============ ON MOUNT ============
    onMount(async () => {
        console.log("🚀 Component mounted");
        
        await loadLatest();
        initChart();
        connectWebSocket();
        
        return () => {
            console.log("🧹 Cleaning up");
            disconnectWebSocket();
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
        <div class="grid grid-cols-2 gap-5">
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
        
        <div class="text-center text-xs text-slate-600 pt-4">
        </div>
    </div>
</div>