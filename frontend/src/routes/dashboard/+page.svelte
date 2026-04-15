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
    
    // ============ YORDAMCHI FUNKSIYALAR ============
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

    // ============ MA'LUMOTLARNI YUKLASH ============
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
            }
        } catch (err) {
            console.error("❌ Failed to load latest:", err);
        } finally {
            isLoading = false;
        }
    }
    
    // ============ CHART YANGILASH ============
    function updateChartData() {
        if (!chart) return;
        
        if (history.length === 0) {
            chart.data.labels = [];
            chart.data.datasets[0].data = [];
            chart.update('none');
            return;
        }
        
        // Vaqt va namlik ma'lumotlarini tayyorlash
        const labels = history.map(h => {
            const date = new Date(h.timestamp);
            return date.toLocaleTimeString('uz-UZ', { 
                hour: '2-digit', 
                minute: '2-digit',
                second: '2-digit'
            });
        });
        
        const values = history.map(h => h.value);
        
        // Chart ma'lumotlarini yangilash
        chart.data.labels = labels;
        chart.data.datasets[0].data = values;
        
        // Smooth yangilanish (animatsiya bilan)
        chart.update('active');
        
        console.log(`📊 Chart updated: ${labels.length} points, Latest: ${values[values.length-1]}%`);
    }
    
    // ============ YANGI MA'LUMOT QO'SHISH ============
    function addNewReading(newData: any) {
        if (!newData || newData.value == null) return;
        
        console.log(`📈 Adding new reading: ${newData.value}% at ${newData.timestamp}`);
        
        // Current ni yangilash
        current = newData;
        lastUpdate = new Date();
        
        // History ni yangilash (immutable)
        const newPoint = {
            value: newData.value,
            timestamp: newData.timestamp || new Date().toISOString()
        };
        
        // Yangi array yaratish (Svelte 5 reaktivligi uchun)
        const updatedHistory = [...history, newPoint];
        
        // Faqat 20 ta nuqta saqlash
        if (updatedHistory.length > 20) {
            history = updatedHistory.slice(-20);
        } else {
            history = updatedHistory;
        }
        
        // Chart ni darhol yangilash (history o'zgarishini kutmasdan)
        if (chart) {
            updateChartData();
        }
        
        console.log(`📊 History size: ${history.length}, Current: ${current.value}%`);
    }
    
    // ============ CHART INIT ============
    function initChart() {
        if (!chartCanvas) {
            console.log("⚠️ Canvas not ready");
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
        
        // Initial ma'lumotlarni yuklash
        if (history.length > 0) {
            updateChartData();
        }
        
        console.log("✅ Chart initialized");
    }
    
    // ============ WEBSOCKET ============
    let ws: WebSocket | null = null;
    let reconnectTimer: any = null;
    let reconnectAttempts = 0;
    
    function connectWebSocket() {
        const token = localStorage.getItem("access");
        if (!token) {
            console.error("❌ No token for WebSocket");
            wsStatus = 'error';
            setTimeout(connectWebSocket, 5000);
            return;
        }
    
        wsStatus = 'connecting';
        reconnectAttempts++;
        
        const wsUrl = ROOT_URL.replace("https://", "wss://").replace("http://", "ws://") + `/ws/updates/?token=${token}`;
        console.log(`🔌 Connecting to WebSocket (attempt ${reconnectAttempts}):`, wsUrl);
    
        try {
            ws = new WebSocket(wsUrl);
            
            ws.onopen = () => {
                console.log("✅ WebSocket connected");
                wsStatus = 'connected';
                reconnectAttempts = 0;
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
                        // Yangi ma'lumotni qo'shish
                        addNewReading(newData);
                    }
                } catch (err) {
                    console.error("❌ Parse error:", err);
                }
            };
        
            ws.onerror = (err) => {
                console.error("❌ WS error:", err);
                wsStatus = 'error';
            };
        
            ws.onclose = (event) => {
                console.log(`❌ WS closed (code: ${event.code}, reason: ${event.reason})`);
                wsStatus = 'error';
                
                // Reconnect with exponential backoff
                if (reconnectTimer) clearTimeout(reconnectTimer);
                const delay = Math.min(30000, 1000 * Math.pow(2, Math.min(reconnectAttempts, 5)));
                console.log(`🔄 Reconnecting in ${delay}ms...`);
                
                reconnectTimer = setTimeout(() => {
                    connectWebSocket();
                }, delay);
            };
        } catch (err) {
            console.error("❌ Failed to create WebSocket:", err);
            wsStatus = 'error';
            setTimeout(connectWebSocket, 5000);
        }
    }

    function disconnectWebSocket() {
        if (reconnectTimer) {
            clearTimeout(reconnectTimer);
            reconnectTimer = null;
        }
        if (ws) {
            ws.close();
            ws = null;
        }
        reconnectAttempts = 0;
    }
    
    // ============ SVELTE 5 REAKTIVLIK ============
    // History o'zgarganda chartni yangilash (backup)
    $effect(() => {
        if (history.length > 0 && chart) {
            console.log("🔄 Effect: History changed, updating chart");
            updateChartData();
        }
    });
    
    // Current o'zgarganda DOM elementlarni yangilash
    $effect(() => {
        if (current?.value != null) {
            // Bu effect faqat UI yangilanishi uchun
            console.log(`🔄 Effect: Current value changed to ${current.value}%`);
        }
    });
    
    // Chart canvas tayyor bo'lganda init
    $effect(() => {
        if (chartCanvas && !chart) {
            console.log("🎨 Canvas ready, initializing chart...");
            setTimeout(() => {
                initChart();
            }, 50);
        }
    });
    
    // ============ ON MOUNT ============
    onMount(async () => {
        console.log("🚀 Component mounted");
        
        await loadLatest();
        
        // WebSocket ulanish
        connectWebSocket();
        
        // Cleanup
        return () => {
            console.log("🧹 Cleaning up components");
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
        
        <!-- Header -->
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

        <!-- Stats Cards -->
        <div class="grid grid-cols-2 gap-5">
            <!-- Namlik Card -->
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
            
            <!-- Holat Card -->
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
        
        <!-- Chart -->
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
            <p>Real-time sensor monitoring | WebSocket connection</p>
        </div>
    </div>
</div>