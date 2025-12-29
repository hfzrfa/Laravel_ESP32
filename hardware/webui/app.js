const $ = (selector) => document.querySelector(selector);
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
            alarm_enabled: document.querySelector(
                '#alarmForm [name="alarm_enabled"]'
            ).checked,
            alarm_time: formData.get("alarm_time"),
        };
    });

    bindForm("outputsForm", "/api/forms/outputs", (formData) => {
        return {
            led_mode: formData.get("led_mode"),
            buzzer_enabled: document.querySelector(
                '#outputsForm [name="buzzer_enabled"]'
            ).checked,
        };
    });

    window.addEventListener("resize", () => drawClimateChart());
});
