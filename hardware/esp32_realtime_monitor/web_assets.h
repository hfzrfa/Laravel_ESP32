#pragma once

#include <pgmspace.h>

const char DASHBOARD_HTML[] PROGMEM = R"rawliteral(<!DOCTYPE html>
<html lang="en" class="h-full">

<head>
    <meta charset="utf-8" />
    <meta name="viewport" content="width=device-width, initial-scale=1" />
    <title>Home Comfort Console</title>
    <link rel="preconnect" href="https://fonts.bunny.net" />
    <link href="https://fonts.bunny.net/css?family=instrument-sans:400,500,600" rel="stylesheet" />
    <link rel="stylesheet" href="/assets/tailwind.css" />
    <script defer src="/assets/app.js"></script>
</head>

<body
    class="min-h-screen bg-gradient-to-br from-zinc-50 via-white to-zinc-100 p-4 text-zinc-900 antialiased dark:from-zinc-950 dark:via-zinc-900 dark:to-zinc-950">
    <div class="mx-auto flex max-w-6xl flex-col gap-6 lg:px-6">
        <header class="flex flex-col gap-4 lg:flex-row lg:items-center lg:justify-between">
            <div>
                <p class="text-xs uppercase tracking-[0.3em] text-zinc-500 dark:text-zinc-400">ESP32 real-time monitor</p>
                <h1 class="mt-2 text-3xl font-semibold text-zinc-900 dark:text-white">Home Comfort Console</h1>
                <p class="mt-1 text-sm text-zinc-500 dark:text-zinc-400">Track DHT11 telemetry, manage displays, and
                    schedule RTC alarms directly from the ESP32 hotspot.</p>
            </div>
            <div class="flex flex-wrap items-center gap-3">
                <span id="device-status"
                    class="badge badge-warn text-sm">
                    <span class="relative flex h-2.5 w-2.5">
                        <span
                            class="absolute inline-flex h-full w-full animate-ping rounded-full bg-current opacity-40"></span>
                        <span class="relative inline-flex h-2.5 w-2.5 rounded-full bg-current"></span>
                    </span>
                    <span id="device-status-text">Waiting for device</span>
                </span>
                <span
                    class="badge border-zinc-200/80 bg-white/80 text-zinc-600 dark:border-zinc-700 dark:bg-zinc-900/60 dark:text-zinc-300">
                    Last ping: <span id="last-ping" class="font-semibold">Never</span>
                </span>
                <span
                    class="badge border-zinc-200/80 bg-white/80 text-zinc-600 dark:border-zinc-700 dark:bg-zinc-900/60 dark:text-zinc-300">
                    IP <span id="ip-address" class="font-semibold">0.0.0.0</span>
                </span>
                <span
                    class="badge border-zinc-200/80 bg-white/80 text-zinc-600 dark:border-zinc-700 dark:bg-zinc-900/60 dark:text-zinc-300">
                    Last sync: <span id="time-sync" class="font-semibold">Never</span>
                </span>
            </div>
        </header>

        <div id="toast" class="hidden rounded-xl border border-emerald-200/60 bg-emerald-50/80 p-4 text-sm text-emerald-900"></div>

        <section class="grid gap-6 lg:grid-cols-3">
            <div class="panel lg:col-span-2">
                <div class="flex items-center justify-between">
                    <div>
                        <p class="panel-title">Live Telemetry</p>
                        <p class="text-xs text-zinc-400">Updates every minute • last update <span id="reading-updated">never</span></p>
                    </div>
                    <div class="text-xs text-zinc-400">
                        Local API served via ESP32
                    </div>
                </div>

                <div class="mt-6 grid gap-4 md:grid-cols-2">
                    <div class="rounded-2xl bg-gradient-to-br from-indigo-500 to-purple-500 p-5 text-white shadow-xl">
                        <p class="text-sm uppercase tracking-wide text-white/80">Temperature</p>
                        <p id="temp-value" class="stat-value mt-3 text-white">--</p>
                        <p class="text-sm text-white/70">Comfort range 21-26°C</p>
                    </div>
                    <div class="rounded-2xl bg-gradient-to-br from-sky-400 to-blue-600 p-5 text-white shadow-xl">
                        <p class="text-sm uppercase tracking-wide text-white/80">Humidity</p>
                        <p id="hum-value" class="stat-value mt-3 text-white">--</p>
                        <p class="text-sm text-white/70">Optimal 40-60%</p>
                    </div>
                    <div
                        class="rounded-2xl border border-zinc-100/70 bg-white/80 p-5 shadow-inner shadow-black/5 dark:border-zinc-800/50 dark:bg-zinc-900/60">
                        <p class="text-sm uppercase tracking-wide text-zinc-500 dark:text-zinc-400">Battery</p>
                        <p id="battery-value" class="stat-value mt-3 text-zinc-900 dark:text-white">--</p>
                        <p class="text-sm text-zinc-500 dark:text-zinc-400">Placeholder estimate</p>
                    </div>
                    <div
                        class="rounded-2xl border border-zinc-100/70 bg-white/80 p-5 shadow-inner shadow-black/5 dark:border-zinc-800/50 dark:bg-zinc-900/60">
                        <p class="text-sm uppercase tracking-wide text-zinc-500 dark:text-zinc-400">Signal</p>
                        <p id="signal-value" class="stat-value mt-3 text-zinc-900 dark:text-white">--</p>
                        <p class="text-sm text-zinc-500 dark:text-zinc-400">STA RSSI</p>
                    </div>
                </div>
            </div>

            <div class="panel flex flex-col gap-4">
                <div>
                    <p class="panel-title">Alarm Timeline</p>
                    <p class="text-sm text-zinc-500 dark:text-zinc-400">RTC based events</p>
                </div>
                <dl class="space-y-3 text-sm text-zinc-600 dark:text-zinc-300">
                    <div class="flex justify-between">
                        <dt>Alarm state</dt>
                        <dd id="alarm-state" class="font-semibold">Disabled</dd>
                    </div>
                    <div class="flex justify-between">
                        <dt>Scheduled time</dt>
                        <dd id="alarm-time" class="font-semibold">—</dd>
                    </div>
                    <div class="flex justify-between">
                        <dt>Last triggered</dt>
                        <dd id="alarm-last" class="font-semibold">Never</dd>
                    </div>
                </dl>
                <div
                    class="rounded-2xl border border-dashed border-zinc-200 p-4 text-xs text-zinc-500 dark:border-zinc-700 dark:text-zinc-400">
                    Alarm acknowledgements are stored locally and triggered entirely by the ESP32.
                </div>
            </div>
        </section>

        <section class="grid gap-6">
            <div class="panel space-y-5">
                <div class="flex flex-wrap items-end justify-between gap-3">
                    <div>
                        <p class="panel-title text-zinc-600 dark:text-zinc-300">Climate analysis</p>
                        <p class="text-sm text-zinc-500 dark:text-zinc-400">Auto-updates every minute with temperature & humidity trends.</p>
                    </div>
                    <div class="text-xs text-zinc-400 dark:text-zinc-300">Local render · No external CDN</div>
                </div>
                <div class="relative">
                    <canvas id="climate-chart" class="h-56 w-full rounded-2xl bg-white/60 dark:bg-zinc-900/60"></canvas>
                    <div id="chart-empty" class="absolute inset-0 flex items-center justify-center rounded-2xl text-sm text-zinc-500 dark:text-zinc-200">
                        Waiting for sensor data...
                    </div>
                </div>
                <div class="flex flex-wrap gap-4 text-xs font-medium text-zinc-500 dark:text-zinc-200">
                    <span class="flex items-center gap-2">
                        <span class="inline-flex h-1.5 w-6 rounded-full bg-gradient-to-r from-orange-400 to-rose-500"></span>
                        Temperature (°C)
                    </span>
                    <span class="flex items-center gap-2">
                        <span class="inline-flex h-1.5 w-6 rounded-full bg-gradient-to-r from-sky-400 to-indigo-500"></span>
                        Humidity (%)
                    </span>
                </div>
            </div>
        </section>

        <section class="grid gap-6 lg:grid-cols-3">
            <form id="displayForm" class="panel space-y-4">
                <div>
                    <p class="panel-title">OLED presets</p>
                    <p class="text-sm text-zinc-500 dark:text-zinc-400">Push copy to the onboard OLED panel. Changes appear on the next minute refresh.</p>
                </div>
                <div class="space-y-2">
                        <label class="text-sm font-medium text-white">OLED mode</label>
                    <select name="oled_mode" class="input-control">
                        <option value="clock">Clock</option>
                        <option value="sensor">Sensor snapshot</option>
                        <option value="custom">Custom text</option>
                    </select>
                    <textarea name="oled_custom_text" rows="3" maxlength="64" class="input-control"
                        placeholder="Up to 64 characters"></textarea>
                    <p class="text-xs text-zinc-400">Need multiple lines? Use \n to insert a break.</p>
                </div>
                <div class="flex justify-end gap-3">
                    <button type="reset" class="btn-ghost">Reset</button>
                    <button type="submit" class="btn-primary">Update displays</button>
                </div>
            </form>

            <form id="alarmForm" class="panel space-y-4">
                <div>
                    <p class="panel-title">RTC alarm</p>
                    <p class="text-sm text-zinc-500 dark:text-zinc-400">Schedule wake-up pulses.</p>
                </div>
                <div class="space-y-2">
                    <label class="inline-flex items-center gap-2 text-sm font-medium">
                        <input type="checkbox" name="alarm_enabled" value="1"
                            class="h-4 w-4 rounded border-zinc-300 text-white focus:ring-indigo-400"
                        Enable alarm
                    </label>
                </div>
                <div class="space-y-2">
                        <label class="text-sm font-medium text-white"
                    <input type="time" name="alarm_time" class="input-control" />
                </div>
                <div
                    class="rounded-2xl border border-dashed border-zinc-200 p-4 text-xs text-zinc-500 dark:border-zinc-700 dark:text-zinc-400">
                    When the ESP32 reaches the alarm time it will buzz/flash and log the event locally.
                </div>
                <div class="flex justify-end">
                    <button type="submit" class="btn-primary">Save alarm</button>
                </div>
            </form>

            <form id="outputsForm" class="panel space-y-4">
                <div>
                    <p class="panel-title">Outputs</p>
                    <p class="text-sm text-zinc-500 dark:text-zinc-400">LED + buzzer policy.</p>
                </div>
                <div class="space-y-2">
                        <label class="text-sm font-medium text-white">LED Mode</label>
                    <select name="led_mode" class="input-control">
                        <option value="auto">Auto (sensors)</option>
                        <option value="on">Force on</option>
                        <option value="off">Force off</option>
                    </select>
                </div>
                <div class="space-y-2">
                    <label class="inline-flex items-center gap-2 text-sm font-medium">
                        <input type="checkbox" name="buzzer_enabled" value="1"
                                class="h-4 w-4 rounded border-zinc-300 text-white focus:ring-indigo-400"
                            />
                        Allow buzzer alerts
                    </label>
                </div>
                <div
                    class="rounded-2xl border border-dashed border-zinc-200 p-4 text-xs text-zinc-500 dark:border-zinc-700 dark:text-zinc-400">
                    Auto mode pulses the LED when humidity &gt; 65% or temperature &gt; 30°C. Manual overrides apply
                    instantly after saving.
                </div>
                <div class="flex justify-end">
                    <button type="submit" class="btn-primary">Sync outputs</button>
                </div>
            </form>
        </section>

        <footer class="pb-10 text-center text-xs text-zinc-500 dark:text-zinc-400">
            Hosted directly from the ESP32 SoftAP. Keep your device connected to <code>ESP32-Monitor</code> for updates.
        </footer>
    </div>
</body>

</html>
)rawliteral";

const char DASHBOARD_JS[] PROGMEM = R"rawliteral(const $ = (selector) => document.querySelector(selector);
const badge = $("#device-status");
const badgeText = $("#device-status-text");
const lastPing = $("#last-ping");
const ipAddress = $("#ip-address");
const timeSync = $("#time-sync");
const toast = $("#toast");
const REFRESH_INTERVAL_MS = 60000;
const HISTORY_ENDPOINT = "/api/telemetry/history";
let toastTimeout;
let chartCanvas;
let chartEmptyState;
let latestHistorySamples = [];

async function fetchJson(url, options) {
    const response = await fetch(url, options);
    if (!response.ok) {
        const text = await response.text();
        throw new Error(text || "Request failed");
    }
    return response.json();
}

function setStatus(connected) {
    if (!badge || !badgeText) return;
    if (connected) {
        badge.classList.remove("badge-warn");
        badge.classList.add("badge-success");
        badgeText.textContent = "Device Online";
    } else {
        badge.classList.remove("badge-success");
        badge.classList.add("badge-warn");
        badgeText.textContent = "Waiting for device";
    }
}

function updateTelemetry(data) {
    $("#temp-value").textContent = data.temperature_c ?? "--";
    $("#hum-value").textContent = data.humidity ?? "--";
    $("#battery-value").textContent = data.battery ?? "--";
    $("#signal-value").textContent = data.signal ?? "--";
    $("#reading-updated").textContent = data.last_reading ?? "never";
    $("#alarm-state").textContent = data.alarm_state;
    $("#alarm-time").textContent = data.alarm_time || "—";
    $("#alarm-last").textContent = data.alarm_last_triggered || "Never";
    ipAddress.textContent = data.ap_ip ?? "0.0.0.0";
    lastPing.textContent = data.last_reading ?? "Never";
    if (timeSync) {
        timeSync.textContent = data.time_last_sync ?? "Never";
    }
    setStatus(true);
}

function fillForms(state) {
    const displayForm = document.getElementById("displayForm");
    const alarmForm = document.getElementById("alarmForm");
    const outputsForm = document.getElementById("outputsForm");

    if (displayForm) {
        displayForm.elements["oled_mode"].value = state.oled_mode;
        displayForm.elements["oled_custom_text"].value =
            state.oled_custom_text ?? "";
    }

    if (alarmForm) {
        alarmForm.elements["alarm_enabled"].checked = !!state.alarm_enabled;
        alarmForm.elements["alarm_time"].value = state.alarm_time ?? "";
    }

    if (outputsForm) {
        outputsForm.elements["led_mode"].value = state.led_mode;
        outputsForm.elements["buzzer_enabled"].checked = !!state.buzzer_enabled;
    }
}

function showToast(message, isError = false) {
    if (!toast) return;
    toast.textContent = message;
    toast.classList.remove("hidden");
    toast.classList.toggle("bg-emerald-50/80", !isError);
    toast.classList.toggle("border-emerald-200/60", !isError);
    toast.classList.toggle("text-emerald-900", !isError);
    toast.classList.toggle("bg-rose-50/80", isError);
    toast.classList.toggle("border-rose-200/60", isError);
    toast.classList.toggle("text-rose-900", isError);
    clearTimeout(toastTimeout);
    toastTimeout = setTimeout(() => toast.classList.add("hidden"), 4000);
}

async function refreshState() {
    try {
        const [telemetry, state, history] = await Promise.all([
            fetchJson("/api/telemetry"),
            fetchJson("/api/state"),
            fetchJson(HISTORY_ENDPOINT).catch(() => []),
        ]);
        updateTelemetry(telemetry);
        fillForms(state);
        updateChart(history ?? []);
    } catch (error) {
        showToast(error.message, true);
        setStatus(false);
    }
}

function updateChart(historySamples) {
    latestHistorySamples = Array.isArray(historySamples)
        ? historySamples.slice(-120)
        : [];
    drawClimateChart();
}

function drawClimateChart() {
    if (!chartCanvas) return;

    const hasData = latestHistorySamples.length > 0;
    if (chartEmptyState) {
        chartEmptyState.classList.toggle("hidden", hasData);
    }
    if (!hasData) {
        const ctx = chartCanvas.getContext("2d");
        ctx.clearRect(0, 0, chartCanvas.width, chartCanvas.height);
        return;
    }

    const labels = latestHistorySamples.map((sample) =>
        formatLabel(sample.epoch)
    );
    const temps = latestHistorySamples.map((sample) => Number(sample.temperature));
    const hums = latestHistorySamples.map((sample) => Number(sample.humidity));

    const ctx = chartCanvas.getContext("2d");
    const width = chartCanvas.clientWidth;
    const height = chartCanvas.clientHeight;
    const dpr = window.devicePixelRatio || 1;
    chartCanvas.width = width * dpr;
    chartCanvas.height = height * dpr;
    ctx.scale(dpr, dpr);
    ctx.clearRect(0, 0, width, height);

    const padding = 32;
    const plotWidth = width - padding * 2;
    const plotHeight = height - padding * 2;
    const dataset = temps.concat(hums).filter((value) => !Number.isNaN(value));
    const fallbackValues = [0, 100];
    const extremes = dataset.length ? dataset : fallbackValues;
    const minValue = Math.min(...extremes);
    const maxValue = Math.max(...extremes);
    const rangePadding = (maxValue - minValue || 10) * 0.15;
    const yMin = minValue - rangePadding;
    const yMax = maxValue + rangePadding;

    const isDark =
        document.documentElement.classList.contains("dark") ||
        window.matchMedia("(prefers-color-scheme: dark)").matches;
    const gridColor = isDark ? "rgba(148, 163, 184, 0.25)" : "rgba(148, 163, 184, 0.35)";

    ctx.strokeStyle = gridColor;
    ctx.lineWidth = 1;
    ctx.setLineDash([4, 6]);
    const gridLines = 4;
    for (let i = 0; i <= gridLines; i++) {
        const y = padding + (plotHeight / gridLines) * i;
        ctx.beginPath();
        ctx.moveTo(padding, y);
        ctx.lineTo(width - padding, y);
        ctx.stroke();
    }
    ctx.setLineDash([]);

    const scaleX = (index, total) => {
        if (total <= 1) return padding + plotWidth / 2;
        return padding + (plotWidth * index) / (total - 1);
    };
    const scaleY = (value) => {
        if (!Number.isFinite(value)) return padding + plotHeight;
        return (
            padding +
            plotHeight -
            ((value - yMin) / (yMax - yMin || 1)) * plotHeight
        );
    };

    const drawSeries = (values, color, fill = false) => {
        ctx.beginPath();
        values.forEach((value, index) => {
            const x = scaleX(index, values.length);
            const y = scaleY(value);
            if (index === 0) {
                ctx.moveTo(x, y);
            } else {
                ctx.lineTo(x, y);
            }
        });
        ctx.strokeStyle = color;
        ctx.lineWidth = 2;
        ctx.stroke();

        if (fill) {
            const gradient = ctx.createLinearGradient(0, padding, 0, height - padding);
            gradient.addColorStop(0, hexToRgba(color, 0.25));
            gradient.addColorStop(1, hexToRgba(color, 0.02));

            ctx.beginPath();
            values.forEach((value, index) => {
                const x = scaleX(index, values.length);
                const y = scaleY(value);
                if (index === 0) {
                    ctx.moveTo(x, y);
                } else {
                    ctx.lineTo(x, y);
                }
            });
            ctx.lineTo(scaleX(values.length - 1, values.length), height - padding);
            ctx.lineTo(scaleX(0, values.length), height - padding);
            ctx.closePath();
            ctx.fillStyle = gradient;
            ctx.fill();
        }
    };

    drawSeries(hums, "#0ea5e9", true);
    drawSeries(temps, "#f97316");

    ctx.fillStyle = isDark ? "#e5e7eb" : "#334155";
    ctx.font = "11px 'Instrument Sans', sans-serif";
    labels.forEach((label, index) => {
        if ((index + 1) % 6 !== 1) return;
        const x = scaleX(index, labels.length);
        ctx.fillText(label, x - 18, height - 6);
    });

    [temps, hums].forEach((values, seriesIndex) => {
        const color = seriesIndex === 0 ? "#f97316" : "#0ea5e9";
        const lastIndex = values.length - 1;
        if (lastIndex < 0) return;
        const x = scaleX(lastIndex, values.length);
        const y = scaleY(values[lastIndex]);
        ctx.beginPath();
        ctx.arc(x, y, 4, 0, Math.PI * 2);
        ctx.fillStyle = color;
        ctx.fill();
    });
}

function formatLabel(epoch) {
    if (!epoch) return "--:--";
    try {
        return new Date(epoch * 1000).toLocaleTimeString("id-ID", {
            hour: "2-digit",
            minute: "2-digit",
            hour12: false,
            timeZone: "Asia/Jakarta",
        });
    } catch (_) {
        return "--:--";
    }
}

function hexToRgba(hex, alpha) {
    const sanitized = hex.replace("#", "");
    const bigint = parseInt(sanitized, 16);
    const r = (bigint >> 16) & 255;
    const g = (bigint >> 8) & 255;
    const b = bigint & 255;
    return `rgba(${r}, ${g}, ${b}, ${alpha})`;
}

function bindForm(formId, endpoint, transform) {
    const form = document.getElementById(formId);
    if (!form) return;
    form.addEventListener("submit", async (event) => {
        event.preventDefault();
        try {
            const payload = transform(new FormData(form));
            await fetchJson(endpoint, {
                method: "POST",
                headers: { "Content-Type": "application/json" },
                body: JSON.stringify(payload),
            });
            showToast("Saved successfully");
            refreshState();
        } catch (error) {
            showToast(error.message, true);
        }
    });
}

function formDataToObject(formData) {
    const obj = {};
    formData.forEach((value, key) => {
        obj[key] = value;
    });
    return obj;
}

window.addEventListener("DOMContentLoaded", () => {
    chartCanvas = document.getElementById("climate-chart");
    chartEmptyState = document.getElementById("chart-empty");

    refreshState();
    setInterval(refreshState, REFRESH_INTERVAL_MS);

    bindForm("displayForm", "/api/forms/displays", (formData) => {
        const payload = formDataToObject(formData);
        payload.oled_custom_text = payload.oled_custom_text?.trim() ?? "";
        return payload;
    });

    bindForm("alarmForm", "/api/forms/alarm", (formData) => {
        return {
            alarm_enabled: document.querySelector('#alarmForm [name="alarm_enabled"]').checked,
            alarm_time: formData.get("alarm_time"),
        };
    });

    bindForm("outputsForm", "/api/forms/outputs", (formData) => {
        return {
            led_mode: formData.get("led_mode"),
            buzzer_enabled: document.querySelector('#outputsForm [name="buzzer_enabled"]').checked,
        };
    });

    window.addEventListener("resize", () => drawClimateChart());
});
)rawliteral";

const char TAILWIND_CSS[] PROGMEM = R"rawliteral(*,:after,:before{--tw-border-spacing-x:0;--tw-border-spacing-y:0;--tw-translate-x:0;--tw-translate-y:0;--tw-rotate:0;--tw-skew-x:0;--tw-skew-y:0;--tw-scale-x:1;--tw-scale-y:1;--tw-pan-x: ;--tw-pan-y: ;--tw-pinch-zoom: ;--tw-scroll-snap-strictness:proximity;--tw-gradient-from-position: ;--tw-gradient-via-position: ;--tw-gradient-to-position: ;--tw-ordinal: ;--tw-slashed-zero: ;--tw-numeric-figure: ;--tw-numeric-spacing: ;--tw-numeric-fraction: ;--tw-ring-inset: ;--tw-ring-offset-width:0px;--tw-ring-offset-color:#fff;--tw-ring-color:rgba(59,130,246,.5);--tw-ring-offset-shadow:0 0 #0000;--tw-ring-shadow:0 0 #0000;--tw-shadow:0 0 #0000;--tw-shadow-colored:0 0 #0000;--tw-blur: ;--tw-brightness: ;--tw-contrast: ;--tw-grayscale: ;--tw-hue-rotate: ;--tw-invert: ;--tw-saturate: ;--tw-sepia: ;--tw-drop-shadow: ;--tw-backdrop-blur: ;--tw-backdrop-brightness: ;--tw-backdrop-contrast: ;--tw-backdrop-grayscale: ;--tw-backdrop-hue-rotate: ;--tw-backdrop-invert: ;--tw-backdrop-opacity: ;--tw-backdrop-saturate: ;--tw-backdrop-sepia: ;--tw-contain-size: ;--tw-contain-layout: ;--tw-contain-paint: ;--tw-contain-style: }::backdrop{--tw-border-spacing-x:0;--tw-border-spacing-y:0;--tw-translate-x:0;--tw-translate-y:0;--tw-rotate:0;--tw-skew-x:0;--tw-skew-y:0;--tw-scale-x:1;--tw-scale-y:1;--tw-pan-x: ;--tw-pan-y: ;--tw-pinch-zoom: ;--tw-scroll-snap-strictness:proximity;--tw-gradient-from-position: ;--tw-gradient-via-position: ;--tw-gradient-to-position: ;--tw-ordinal: ;--tw-slashed-zero: ;--tw-numeric-figure: ;--tw-numeric-spacing: ;--tw-numeric-fraction: ;--tw-ring-inset: ;--tw-ring-offset-width:0px;--tw-ring-offset-color:#fff;--tw-ring-color:rgba(59,130,246,.5);--tw-ring-offset-shadow:0 0 #0000;--tw-ring-shadow:0 0 #0000;--tw-shadow:0 0 #0000;--tw-shadow-colored:0 0 #0000;--tw-blur: ;--tw-brightness: ;--tw-contrast: ;--tw-grayscale: ;--tw-hue-rotate: ;--tw-invert: ;--tw-saturate: ;--tw-sepia: ;--tw-drop-shadow: ;--tw-backdrop-blur: ;--tw-backdrop-brightness: ;--tw-backdrop-contrast: ;--tw-backdrop-grayscale: ;--tw-backdrop-hue-rotate: ;--tw-backdrop-invert: ;--tw-backdrop-opacity: ;--tw-backdrop-saturate: ;--tw-backdrop-sepia: ;--tw-contain-size: ;--tw-contain-layout: ;--tw-contain-paint: ;--tw-contain-style: }/*! tailwindcss v3.4.15 | MIT License | https://tailwindcss.com*/*,:after,:before{box-sizing:border-box;border:0 solid #e5e7eb}:after,:before{--tw-content:""}:host,html{line-height:1.5;-webkit-text-size-adjust:100%;-moz-tab-size:4;-o-tab-size:4;tab-size:4;font-family:ui-sans-serif,system-ui,sans-serif,Apple Color Emoji,Segoe UI Emoji,Segoe UI Symbol,Noto Color Emoji;font-feature-settings:normal;font-variation-settings:normal;-webkit-tap-highlight-color:transparent}body{margin:0;line-height:inherit}hr{height:0;color:inherit;border-top-width:1px}abbr:where([title]){-webkit-text-decoration:underline dotted;text-decoration:underline dotted}h1,h2,h3,h4,h5,h6{font-size:inherit;font-weight:inherit}a{color:inherit;text-decoration:inherit}b,strong{font-weight:bolder}code,kbd,pre,samp{font-family:ui-monospace,SFMono-Regular,Menlo,Monaco,Consolas,Liberation Mono,Courier New,monospace;font-feature-settings:normal;font-variation-settings:normal;font-size:1em}small{font-size:80%}sub,sup{font-size:75%;line-height:0;position:relative;vertical-align:baseline}sub{bottom:-.25em}sup{top:-.5em}table{text-indent:0;border-color:inherit;border-collapse:collapse}button,input,optgroup,select,textarea{font-family:inherit;font-feature-settings:inherit;font-variation-settings:inherit;font-size:100%;font-weight:inherit;line-height:inherit;letter-spacing:inherit;color:inherit;margin:0;padding:0}button,select{text-transform:none}button,input:where([type=button]),input:where([type=reset]),input:where([type=submit]){-webkit-appearance:button;background-color:transparent;background-image:none}:-moz-focusring{outline:auto}:-moz-ui-invalid{box-shadow:none}progress{vertical-align:baseline}::-webkit-inner-spin-button,::-webkit-outer-spin-button{height:auto}[type=search]{-webkit-appearance:textfield;outline-offset:-2px}::-webkit-search-decoration{-webkit-appearance:none}::-webkit-file-upload-button{-webkit-appearance:button;font:inherit}summary{display:list-item}blockquote,dd,dl,figure,h1,h2,h3,h4,h5,h6,hr,p,pre{margin:0}fieldset{margin:0}fieldset,legend{padding:0}menu,ol,ul{list-style:none;margin:0;padding:0}dialog{padding:0}textarea{resize:vertical}input::-moz-placeholder,textarea::-moz-placeholder{opacity:1;color:#9ca3af}input::placeholder,textarea::placeholder{opacity:1;color:#9ca3af}[role=button],button{cursor:pointer}:disabled{cursor:default}audio,canvas,embed,iframe,img,object,svg,video{display:block;vertical-align:middle}img,video{max-width:100%;height:auto}[hidden]:where(:not([hidden=until-found])){display:none}body{font-family:Instrument Sans,ui-sans-serif,system-ui,sans-serif}.panel{border-radius:1rem;border-width:1px;border-color:hsla(240,5%,96%,.8);background-color:hsla(0,0%,100%,.8);padding:1.5rem;--tw-shadow:0 20px 25px -5px rgba(0,0,0,.1),0 8px 10px -6px rgba(0,0,0,.1);--tw-shadow-colored:0 20px 25px -5px var(--tw-shadow-color),0 8px 10px -6px var(--tw-shadow-color);box-shadow:var(--tw-ring-offset-shadow,0 0 #0000),var(--tw-ring-shadow,0 0 #0000),var(--tw-shadow);--tw-shadow-color:rgba(0,0,0,.05);--tw-shadow:var(--tw-shadow-colored);--tw-backdrop-blur:blur(8px);-webkit-backdrop-filter:var(--tw-backdrop-blur) var(--tw-backdrop-brightness) var(--tw-backdrop-contrast) var(--tw-backdrop-grayscale) var(--tw-backdrop-hue-rotate) var(--tw-backdrop-invert) var(--tw-backdrop-opacity) var(--tw-backdrop-saturate) var(--tw-backdrop-sepia);backdrop-filter:var(--tw-backdrop-blur) var(--tw-backdrop-brightness) var(--tw-backdrop-contrast) var(--tw-backdrop-grayscale) var(--tw-backdrop-hue-rotate) var(--tw-backdrop-invert) var(--tw-backdrop-opacity) var(--tw-backdrop-saturate) var(--tw-backdrop-sepia)}@media (prefers-color-scheme:dark){.panel{--tw-border-opacity:1;border-color:rgb(39 39 42/var(--tw-border-opacity,1));background-color:rgba(24,24,27,.8);--tw-shadow-color:rgba(0,0,0,.4);--tw-shadow:var(--tw-shadow-colored)}}.panel-title{font-size:.875rem;line-height:1.25rem;font-weight:600;text-transform:uppercase;letter-spacing:.025em;--tw-text-opacity:1;color:rgb(113 113 122/var(--tw-text-opacity,1))}@media (prefers-color-scheme:dark){.panel-title{--tw-text-opacity:1;color:rgb(161 161 170/var(--tw-text-opacity,1))}}.stat-value{font-size:2.25rem;line-height:2.5rem;font-weight:600;--tw-text-opacity:1;color:rgb(24 24 27/var(--tw-text-opacity,1))}@media (prefers-color-scheme:dark){.stat-value{--tw-text-opacity:1;color:rgb(255 255 255/var(--tw-text-opacity,1))}}.badge{display:inline-flex;align-items:center;gap:.5rem;border-radius:9999px;border-width:1px;padding:.25rem .75rem;font-size:.75rem;line-height:1rem;font-weight:500}.badge-warn{border-color:hsla(48,97%,77%,.6);background-color:rgba(255,251,235,.8);--tw-text-opacity:1;color:rgb(180 83 9/var(--tw-text-opacity,1))}@media (prefers-color-scheme:dark){.badge-warn{border-color:rgba(120,53,15,.6);background-color:rgba(120,53,15,.4);--tw-text-opacity:1;color:rgb(253 230 138/var(--tw-text-opacity,1))}}.btn-primary{display:inline-flex;align-items:center;justify-content:center;border-radius:.75rem;background-image:linear-gradient(to right,var(--tw-gradient-stops));--tw-gradient-from:#6366f1 var(--tw-gradient-from-position);--tw-gradient-to:rgba(99,102,241,0) var(--tw-gradient-to-position);--tw-gradient-stops:var(--tw-gradient-from),var(--tw-gradient-to);--tw-gradient-to:#a855f7 var(--tw-gradient-to-position);padding:.5rem 1rem;font-size:.875rem;line-height:1.25rem;font-weight:600;--tw-text-opacity:1;color:rgb(255 255 255/var(--tw-text-opacity,1));--tw-shadow:0 10px 15px -3px rgba(0,0,0,.1),0 4px 6px -4px rgba(0,0,0,.1);--tw-shadow-colored:0 10px 15px -3px var(--tw-shadow-color),0 4px 6px -4px var(--tw-shadow-color);box-shadow:var(--tw-ring-offset-shadow,0 0 #0000),var(--tw-ring-shadow,0 0 #0000),var(--tw-shadow);--tw-shadow-color:rgba(99,102,241,.3);--tw-shadow:var(--tw-shadow-colored);transition-property:color,background-color,border-color,text-decoration-color,fill,stroke,opacity,box-shadow,transform,filter,-webkit-backdrop-filter;transition-property:color,background-color,border-color,text-decoration-color,fill,stroke,opacity,box-shadow,transform,filter,backdrop-filter;transition-property:color,background-color,border-color,text-decoration-color,fill,stroke,opacity,box-shadow,transform,filter,backdrop-filter,-webkit-backdrop-filter;transition-timing-function:cubic-bezier(.4,0,.2,1);transition-duration:.15s}.btn-primary:hover{opacity:.9}.btn-primary:focus-visible{outline-width:2px;outline-offset:2px;outline-color:#6366f1}.btn-ghost{display:inline-flex;align-items:center;justify-content:center;border-radius:.75rem;border-width:1px;--tw-border-opacity:1;border-color:rgb(228 228 231/var(--tw-border-opacity,1));padding:.5rem 1rem;font-size:.875rem;line-height:1.25rem;font-weight:600;--tw-text-opacity:1;color:rgb(63 63 70/var(--tw-text-opacity,1));transition-property:color,background-color,border-color,text-decoration-color,fill,stroke,opacity,box-shadow,transform,filter,-webkit-backdrop-filter;transition-property:color,background-color,border-color,text-decoration-color,fill,stroke,opacity,box-shadow,transform,filter,backdrop-filter;transition-property:color,background-color,border-color,text-decoration-color,fill,stroke,opacity,box-shadow,transform,filter,backdrop-filter,-webkit-backdrop-filter;transition-timing-function:cubic-bezier(.4,0,.2,1);transition-duration:.15s}.btn-ghost:hover{--tw-bg-opacity:1;background-color:rgb(250 250 250/var(--tw-bg-opacity,1))}@media (prefers-color-scheme:dark){.btn-ghost{--tw-border-opacity:1;border-color:rgb(63 63 70/var(--tw-border-opacity,1));--tw-text-opacity:1;color:rgb(228 228 231/var(--tw-text-opacity,1))}.btn-ghost:hover{--tw-bg-opacity:1;background-color:rgb(39 39 42/var(--tw-bg-opacity,1))}}.input-control{width:100%;border-radius:.75rem;border-width:1px;--tw-border-opacity:1;border-color:rgb(228 228 231/var(--tw-border-opacity,1));background-color:hsla(0,0%,100%,.7);padding:.5rem .75rem;font-size:.875rem;line-height:1.25rem;--tw-text-opacity:1;color:rgb(24 24 27/var(--tw-text-opacity,1));--tw-shadow:inset 0 2px 4px 0 rgba(0,0,0,.05);--tw-shadow-colored:inset 0 2px 4px 0 var(--tw-shadow-color);box-shadow:var(--tw-ring-offset-shadow,0 0 #0000),var(--tw-ring-shadow,0 0 #0000),var(--tw-shadow);--tw-shadow-color:rgba(0,0,0,.05);--tw-shadow:var(--tw-shadow-colored);transition-property:color,background-color,border-color,text-decoration-color,fill,stroke,opacity,box-shadow,transform,filter,-webkit-backdrop-filter;transition-property:color,background-color,border-color,text-decoration-color,fill,stroke,opacity,box-shadow,transform,filter,backdrop-filter;transition-property:color,background-color,border-color,text-decoration-color,fill,stroke,opacity,box-shadow,transform,filter,backdrop-filter,-webkit-backdrop-filter;transition-timing-function:cubic-bezier(.4,0,.2,1);transition-duration:.15s}.input-control:focus{--tw-border-opacity:1;border-color:rgb(129 140 248/var(--tw-border-opacity,1));--tw-ring-offset-shadow:var(--tw-ring-inset) 0 0 0 var(--tw-ring-offset-width) var(--tw-ring-offset-color);--tw-ring-shadow:var(--tw-ring-inset) 0 0 0 calc(2px + var(--tw-ring-offset-width)) var(--tw-ring-color);box-shadow:var(--tw-ring-offset-shadow),var(--tw-ring-shadow),var(--tw-shadow,0 0 #0000);--tw-ring-opacity:1;--tw-ring-color:rgb(224 231 255/var(--tw-ring-opacity,1))}@media (prefers-color-scheme:dark){.input-control{--tw-border-opacity:1;border-color:rgb(63 63 70/var(--tw-border-opacity,1));background-color:rgba(24,24,27,.6);--tw-text-opacity:1;color:rgb(255 255 255/var(--tw-text-opacity,1))}.input-control:focus{--tw-border-opacity:1;border-color:rgb(129 140 248/var(--tw-border-opacity,1));--tw-ring-color:rgba(49,46,129,.6)}}.absolute{position:absolute}.relative{position:relative}.mx-auto{margin-left:auto;margin-right:auto}.mt-1{margin-top:.25rem}.mt-2{margin-top:.5rem}.mt-3{margin-top:.75rem}.mt-6{margin-top:1.5rem}.flex{display:flex}.inline-flex{display:inline-flex}.grid{display:grid}.hidden{display:none}.h-2\.5{height:.625rem}.h-4{height:1rem}.h-full{height:100%}.min-h-screen{min-height:100vh}.w-2\.5{width:.625rem}.w-4{width:1rem}.w-full{width:100%}.max-w-6xl{max-width:72rem}@keyframes ping{75%,to{transform:scale(2);opacity:0}}.animate-ping{animation:ping 1s cubic-bezier(0,0,.2,1) infinite}.flex-col{flex-direction:column}.flex-wrap{flex-wrap:wrap}.items-center{align-items:center}.justify-end{justify-content:flex-end}.justify-between{justify-content:space-between}.gap-2{gap:.5rem}.gap-3{gap:.75rem}.gap-4{gap:1rem}.gap-6{gap:1.5rem}.space-y-2>:not([hidden])~:not([hidden]){--tw-space-y-reverse:0;margin-top:calc(.5rem*(1 - var(--tw-space-y-reverse)));margin-bottom:calc(.5rem*var(--tw-space-y-reverse))}.space-y-3>:not([hidden])~:not([hidden]){--tw-space-y-reverse:0;margin-top:calc(.75rem*(1 - var(--tw-space-y-reverse)));margin-bottom:calc(.75rem*var(--tw-space-y-reverse))}.space-y-4>:not([hidden])~:not([hidden]){--tw-space-y-reverse:0;margin-top:calc(1rem*(1 - var(--tw-space-y-reverse)));margin-bottom:calc(1rem*var(--tw-space-y-reverse))}.rounded{border-radius:.25rem}.rounded-2xl{border-radius:1rem}.rounded-full{border-radius:9999px}.rounded-xl{border-radius:.75rem}.border{border-width:1px}.border-dashed{border-style:dashed}.border-emerald-200\/60{border-color:rgba(167,243,208,.6)}.border-zinc-100\/70{border-color:hsla(240,5%,96%,.7)}.border-zinc-200{--tw-border-opacity:1;border-color:rgb(228 228 231/var(--tw-border-opacity,1))}.border-zinc-200\/80{border-color:hsla(240,6%,90%,.8)}.border-zinc-300{--tw-border-opacity:1;border-color:rgb(212 212 216/var(--tw-border-opacity,1))}.bg-current{background-color:currentColor}.bg-emerald-50\/80{background-color:rgba(236,253,245,.8)}.bg-white\/80{background-color:hsla(0,0%,100%,.8)}.bg-gradient-to-br{background-image:linear-gradient(to bottom right,var(--tw-gradient-stops))}.from-indigo-500{--tw-gradient-from:#6366f1 var(--tw-gradient-from-position);--tw-gradient-to:rgba(99,102,241,0) var(--tw-gradient-to-position);--tw-gradient-stops:var(--tw-gradient-from),var(--tw-gradient-to)}.from-sky-400{--tw-gradient-from:#38bdf8 var(--tw-gradient-from-position);--tw-gradient-to:rgba(56,189,248,0) var(--tw-gradient-to-position);--tw-gradient-stops:var(--tw-gradient-from),var(--tw-gradient-to)}.from-zinc-50{--tw-gradient-from:#fafafa var(--tw-gradient-from-position);--tw-gradient-to:hsla(0,0%,98%,0) var(--tw-gradient-to-position);--tw-gradient-stops:var(--tw-gradient-from),var(--tw-gradient-to)}.via-white{--tw-gradient-to:hsla(0,0%,100%,0) var(--tw-gradient-to-position);--tw-gradient-stops:var(--tw-gradient-from),#fff var(--tw-gradient-via-position),var(--tw-gradient-to)}.to-blue-600{--tw-gradient-to:#2563eb var(--tw-gradient-to-position)}.to-purple-500{--tw-gradient-to:#a855f7 var(--tw-gradient-to-position)}.to-zinc-100{--tw-gradient-to:#f4f4f5 var(--tw-gradient-to-position)}.p-4{padding:1rem}.p-5{padding:1.25rem}.pb-10{padding-bottom:2.5rem}.text-center{text-align:center}.text-3xl{font-size:1.875rem;line-height:2.25rem}.text-sm{font-size:.875rem;line-height:1.25rem}.text-xs{font-size:.75rem;line-height:1rem}.font-medium{font-weight:500}.font-semibold{font-weight:600}.uppercase{text-transform:uppercase}.tracking-\[0\.3em\]{letter-spacing:.3em}.tracking-wide{letter-spacing:.025em}.text-emerald-900{--tw-text-opacity:1;color:rgb(6 78 59/var(--tw-text-opacity,1))}.text-indigo-500{--tw-text-opacity:1;color:rgb(99 102 241/var(--tw-text-opacity,1))}.text-white{--tw-text-opacity:1;color:rgb(255 255 255/var(--tw-text-opacity,1))}.text-white\/70{color:hsla(0,0%,100%,.7)}.text-white\/80{color:hsla(0,0%,100%,.8)}.text-zinc-400{--tw-text-opacity:1;color:rgb(161 161 170/var(--tw-text-opacity,1))}.text-zinc-500{--tw-text-opacity:1;color:rgb(113 113 122/var(--tw-text-opacity,1))}.text-zinc-600{--tw-text-opacity:1;color:rgb(82 82 91/var(--tw-text-opacity,1))}.text-zinc-900{--tw-text-opacity:1;color:rgb(24 24 27/var(--tw-text-opacity,1))}.antialiased{-webkit-font-smoothing:antialiased;-moz-osx-font-smoothing:grayscale}.opacity-40{opacity:.4}.shadow-inner{--tw-shadow:inset 0 2px 4px 0 rgba(0,0,0,.05);--tw-shadow-colored:inset 0 2px 4px 0 var(--tw-shadow-color)}.shadow-inner,.shadow-xl{box-shadow:var(--tw-ring-offset-shadow,0 0 #0000),var(--tw-ring-shadow,0 0 #0000),var(--tw-shadow)}.shadow-xl{--tw-shadow:0 20px 25px -5px rgba(0,0,0,.1),0 8px 10px -6px rgba(0,0,0,.1);--tw-shadow-colored:0 20px 25px -5px var(--tw-shadow-color),0 8px 10px -6px var(--tw-shadow-color)}.shadow-black\/5{--tw-shadow-color:rgba(0,0,0,.05);--tw-shadow:var(--tw-shadow-colored)}.focus\:ring-indigo-400:focus{--tw-ring-opacity:1;--tw-ring-color:rgb(129 140 248/var(--tw-ring-opacity,1))}@media (min-width:768px){.md\:grid-cols-2{grid-template-columns:repeat(2,minmax(0,1fr))}}@media (min-width:1024px){.lg\:col-span-2{grid-column:span 2/span 2}.lg\:grid-cols-3{grid-template-columns:repeat(3,minmax(0,1fr))}.lg\:flex-row{flex-direction:row}.lg\:items-center{align-items:center}.lg\:justify-between{justify-content:space-between}.lg\:px-6{padding-left:1.5rem;padding-right:1.5rem}}@media (prefers-color-scheme:dark){.dark\:border-zinc-700{--tw-border-opacity:1;border-color:rgb(63 63 70/var(--tw-border-opacity,1))}.dark\:border-zinc-800\/50{border-color:rgba(39,39,42,.5)}.dark\:bg-zinc-900\/60{background-color:rgba(24,24,27,.6)}.dark\:from-zinc-950{--tw-gradient-from:#09090b var(--tw-gradient-from-position);--tw-gradient-to:rgba(9,9,11,0) var(--tw-gradient-to-position);--tw-gradient-stops:var(--tw-gradient-from),var(--tw-gradient-to)}.dark\:via-zinc-900{--tw-gradient-to:rgba(24,24,27,0) var(--tw-gradient-to-position);--tw-gradient-stops:var(--tw-gradient-from),#18181b var(--tw-gradient-via-position),var(--tw-gradient-to)}.dark\:to-zinc-950{--tw-gradient-to:#09090b var(--tw-gradient-to-position)}.dark\:text-white{--tw-text-opacity:1;color:rgb(255 255 255/var(--tw-text-opacity,1))}.dark\:text-zinc-300{--tw-text-opacity:1;color:rgb(212 212 216/var(--tw-text-opacity,1))}.dark\:text-zinc-400{--tw-text-opacity:1;color:rgb(161 161 170/var(--tw-text-opacity,1))}})rawliteral";
