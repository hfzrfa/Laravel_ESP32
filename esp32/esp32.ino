#include <WiFi.h>
#include <ArduinoJson.h>
#include <DHT.h>
#include <Wire.h>
#include <Adafruit_SSD1306.h>
#include <RTClib.h>
#include <WebServer.h>
#include <time.h>
#include <stdlib.h>
#include "web_assets.h"

/*** ---------- Wi-Fi configuration (update before flashing) ---------- ***/
constexpr const char *STA_SSID = "05Room";             // Fill with your Wi-Fi SSID
constexpr const char *STA_PASSWORD = "10042006";         // Fill with your Wi-Fi password
constexpr const char *AP_SSID = "ESP32-Monitor"; // Hotspot served by the ESP32
constexpr const char *AP_PASSWORD = "monitor123";

/*** ---------- Hardware pins ---------- ***/
constexpr uint8_t DHT_PIN = 4;
constexpr uint8_t BUZZER_PIN = 16;
constexpr uint8_t LED_PIN = 17;

/*** ---------- Display + sensor config ---------- ***/
constexpr uint8_t OLED_I2C_ADDRESS = 0x3C;
constexpr uint8_t OLED_WIDTH = 128;
constexpr uint8_t OLED_HEIGHT = 64;
constexpr float TEMP_THRESHOLD_AUTO = 30.0f;
constexpr float HUM_THRESHOLD_AUTO = 65.0f;

/*** ---------- Timers (ms) ---------- ***/
constexpr uint32_t READING_INTERVAL_MS = 60000;        // DHT sample cadence aligned with UI
constexpr uint32_t DISPLAY_REFRESH_INTERVAL_MS = 1000; // Update OLED text each second
constexpr uint32_t AP_HEALTH_CHECK_INTERVAL_MS = 15000; // Re-assert SoftAP every 15 seconds
constexpr uint32_t TIME_SYNC_INTERVAL_MS = 3600000;     // Sync RTC once per hour
constexpr uint32_t TIME_SYNC_MIN_RETRY_MS = 60000;      // Retry at most once per minute
constexpr const char *TZ_INFO = "WIB-7";               // Asia/Jakarta timezone descriptor
constexpr const char *NTP_SERVER_1 = "pool.ntp.org";
constexpr const char *NTP_SERVER_2 = "time.nist.gov";
constexpr size_t TELEMETRY_HISTORY_SIZE = 120;          // 2 hours of 1-minute samples

struct DeviceStatePayload
{
    String oledMode = "sensor";
    String oledText = "";
    String ledMode = "auto";
    bool buzzerEnabled = false;
    bool alarmEnabled = false;
    String alarmTime = ""; // HH:MM
};

DHT dht(DHT_PIN, DHT11);
Adafruit_SSD1306 oled(OLED_WIDTH, OLED_HEIGHT, &Wire, -1);
RTC_DS3231 rtc;
DeviceStatePayload currentState;
WebServer localServer(80);

struct TelemetrySample
{
    uint32_t epoch = 0;
    float temperature = NAN;
    float humidity = NAN;
    bool valid = false;
};

float lastTemperature = NAN;
float lastHumidity = NAN;
unsigned long lastReadingAt = 0;
unsigned long lastDisplayRefreshAt = 0;
unsigned long lastTelemetryAt = 0;
unsigned long lastAlarmTriggeredAt = 0;
bool alarmLatched = false;
float lastBatteryLevel = 100.0f;
int16_t lastSignalStrength = 0;
unsigned long lastApHealthCheckAt = 0;
unsigned long lastTimeSyncAt = 0;
unsigned long lastTimeSyncAttemptAt = 0;
TelemetrySample telemetryHistory[TELEMETRY_HISTORY_SIZE];
size_t telemetryHistoryCount = 0;
size_t telemetryHistoryIndex = 0;

void setupAccessPoint();
void registerLocalEndpoints();
void sendStateSnapshot();
void handleDisplayForm();
void handleAlarmForm();
void handleOutputsForm();
void updateDisplays();
void updateOledScene();
void drawAlarmBadge();
void applyOutputs();
void handleAlarm();
void triggerAlarmFeedback();
bool shouldTriggerAlarm();
String getCurrentClockString();
String describeTimeSince(unsigned long timestamp);
String buildTelemetryJson();
String buildTelemetryHistoryJson();
bool loadJsonBody(DynamicJsonDocument &doc);
void maintainAccessPoint(unsigned long now);
void syncClockIfNeeded(unsigned long now);
bool syncClockFromNtp();
bool fetchNtpTime(DateTime &timestamp);
void recordTelemetrySample(float temperature, float humidity);

void setup()
{
    pinMode(LED_PIN, OUTPUT);
    pinMode(BUZZER_PIN, OUTPUT);
    digitalWrite(LED_PIN, LOW);
    digitalWrite(BUZZER_PIN, LOW);

    Serial.begin(115200);
    delay(250);

    dht.begin();
    Wire.begin();

    if (!oled.begin(SSD1306_SWITCHCAPVCC, OLED_I2C_ADDRESS))
    {
        Serial.println(F("[ERR] OLED init failed"));
    }
    else
    {
        oled.clearDisplay();
        oled.setTextColor(SSD1306_WHITE);
        oled.setTextSize(1);
        oled.setCursor(0, 0);
        oled.println("ESP32 Monitor");
        oled.display();
    }

    if (!rtc.begin())
    {
        Serial.println(F("[ERR] RTC not detected"));
    }

    setupAccessPoint();
    registerLocalEndpoints();
    localServer.begin();

    syncClockIfNeeded(millis());
    updateDisplays();
}

void loop()
{
    localServer.handleClient();

    unsigned long now = millis();

    maintainAccessPoint(now);
    syncClockIfNeeded(now);

    if (now - lastReadingAt >= READING_INTERVAL_MS)
    {
        float temperature = dht.readTemperature();
        float humidity = dht.readHumidity();

        if (isnan(temperature) || isnan(humidity))
        {
            Serial.println(F("[WARN] Failed to read DHT11"));
        }
        else
        {
            lastTemperature = temperature;
            lastHumidity = humidity;
            lastTelemetryAt = now;

            wifi_mode_t mode = WiFi.getMode();
            bool staConnected = (mode == WIFI_MODE_STA || mode == WIFI_MODE_APSTA) && WiFi.isConnected();
            lastSignalStrength = staConnected ? WiFi.RSSI() : 0;
            applyOutputs();
            recordTelemetrySample(temperature, humidity);
        }
        lastReadingAt = now;
    }

    if (now - lastDisplayRefreshAt >= DISPLAY_REFRESH_INTERVAL_MS)
    {
        updateDisplays();
        lastDisplayRefreshAt = now;
    }

    handleAlarm();
}

void setupAccessPoint()
{
    WiFi.persistent(false);   // Avoid writing Wi-Fi config to flash repeatedly
    WiFi.disconnect(true);    // Drop any lingering STA sessions
    WiFi.mode(WIFI_MODE_APSTA); // Keep AP active while allowing STA joins for NTP
    WiFi.setAutoReconnect(false);
    WiFi.setSleep(false);
    WiFi.setTxPower(WIFI_POWER_19_5dBm);

    IPAddress apIp(192, 168, 4, 1);
    IPAddress gateway(192, 168, 4, 1);
    IPAddress subnet(255, 255, 255, 0);
    if (!WiFi.softAPConfig(apIp, gateway, subnet))
    {
        Serial.println(F("[AP] Failed to configure static IP"));
    }

    bool apOk = WiFi.softAP(AP_SSID, AP_PASSWORD, 6, 0, 4);
    if (apOk)
    {
        IPAddress currentIp = WiFi.softAPIP();
        Serial.print(F("[AP] SSID: "));
        Serial.print(AP_SSID);
        Serial.print(F(" | IP: "));
        Serial.println(currentIp);
    }
    else
    {
        Serial.println(F("[AP] Failed to start SoftAP"));
    }

    lastApHealthCheckAt = millis();
}

void maintainAccessPoint(unsigned long now)
{
    if (now - lastApHealthCheckAt < AP_HEALTH_CHECK_INTERVAL_MS)
    {
        return;
    }

    lastApHealthCheckAt = now;

    wifi_mode_t mode = WiFi.getMode();
    bool apActive = (mode == WIFI_MODE_AP || mode == WIFI_MODE_APSTA);
    if (!apActive)
    {
        Serial.println(F("[AP] Mode drift detected, restarting SoftAP"));
        setupAccessPoint();
        return;
    }

    IPAddress currentIp = WiFi.softAPIP();
    if (currentIp[0] == 0 && currentIp[1] == 0 && currentIp[2] == 0 && currentIp[3] == 0)
    {
        Serial.println(F("[AP] Invalid SoftAP IP detected, restarting"));
        setupAccessPoint();
    }
}

void syncClockIfNeeded(unsigned long now)
{
    if (String(STA_SSID).isEmpty())
    {
        return;
    }

    if (lastTimeSyncAt != 0 && now - lastTimeSyncAt < TIME_SYNC_INTERVAL_MS)
    {
        return;
    }

    if (now - lastTimeSyncAttemptAt < TIME_SYNC_MIN_RETRY_MS)
    {
        return;
    }

    if (syncClockFromNtp())
    {
            lastTimeSyncAt = now; // Ensure lastTimeSyncAt uses scheduler timestamp
    }
    lastTimeSyncAttemptAt = now;
}

bool syncClockFromNtp()
{
    Serial.println(F("[TIME] Starting Wi-Fi sync for RTC"));

    WiFi.mode(WIFI_MODE_APSTA);
    WiFi.begin(STA_SSID, STA_PASSWORD);

    uint32_t start = millis();
    while (WiFi.status() != WL_CONNECTED && millis() - start < 15000)
    {
        delay(250);
        Serial.print('.');
    }
    Serial.println();

    if (WiFi.status() != WL_CONNECTED)
    {
        Serial.println(F("[TIME] STA connection failed"));
        WiFi.disconnect(false, true);
        return false;
    }

    DateTime networkTimestamp;
    bool ntpOk = fetchNtpTime(networkTimestamp);

    WiFi.disconnect(false, true); // Drop STA connection only, keep AP alive

    if (!ntpOk)
    {
        Serial.println(F("[TIME] NTP fetch failed"));
        return false;
    }

    rtc.adjust(networkTimestamp);
    Serial.print(F("[TIME] RTC synced to "));
    Serial.println(getCurrentClockString());
    return true;
}

bool fetchNtpTime(DateTime &timestamp)
{
    setenv("TZ", TZ_INFO, 1);
    tzset();
    configTime(0, 0, NTP_SERVER_1, NTP_SERVER_2);

    struct tm timeinfo;
    if (!getLocalTime(&timeinfo, 10000))
    {
        Serial.println(F("[TIME] Failed to obtain NTP time"));
        return false;
    }

    timestamp = DateTime(timeinfo.tm_year + 1900,
                         timeinfo.tm_mon + 1,
                         timeinfo.tm_mday,
                         timeinfo.tm_hour,
                         timeinfo.tm_min,
                         timeinfo.tm_sec);
    return true;
}

void registerLocalEndpoints()
{
    localServer.on("/", HTTP_GET, []() {
        localServer.send_P(200, "text/html", DASHBOARD_HTML);
    });

    localServer.on("/assets/tailwind.css", HTTP_GET, []() {
        localServer.send_P(200, "text/css", TAILWIND_CSS);
    });

    localServer.on("/assets/app.js", HTTP_GET, []() {
        localServer.send_P(200, "application/javascript", DASHBOARD_JS);
    });

    localServer.on("/api/telemetry", HTTP_GET, []() {
        String payload = buildTelemetryJson();
        localServer.send(200, "application/json", payload);
    });

    localServer.on("/api/telemetry/history", HTTP_GET, []() {
        String payload = buildTelemetryHistoryJson();
        localServer.send(200, "application/json", payload);
    });

    localServer.on("/api/state", HTTP_GET, []() {
        sendStateSnapshot();
    });

    localServer.on("/api/forms/displays", HTTP_POST, handleDisplayForm);
    localServer.on("/api/forms/alarm", HTTP_POST, handleAlarmForm);
    localServer.on("/api/forms/outputs", HTTP_POST, handleOutputsForm);
}

String buildTelemetryJson()
{
    DynamicJsonDocument json(512);
    json["temperature_c"] = isnan(lastTemperature) ? "--" : String(lastTemperature, 1) + "°C";
    json["humidity"] = isnan(lastHumidity) ? "--" : String(lastHumidity, 1) + "%";
    json["battery"] = String(lastBatteryLevel, 0) + "%";
    wifi_mode_t mode = WiFi.getMode();
    bool staConnected = (mode == WIFI_MODE_STA || mode == WIFI_MODE_APSTA) && WiFi.isConnected();
    String signalSummary = staConnected ? String(lastSignalStrength) + " dBm"
                                        : String(WiFi.softAPgetStationNum()) + " clients";
    json["signal"] = signalSummary;
    json["clock"] = getCurrentClockString();
    json["last_reading"] = describeTimeSince(lastTelemetryAt);
    json["alarm_state"] = currentState.alarmEnabled ? "Armed" : "Disabled";
    json["alarm_time"] = currentState.alarmTime;
    json["alarm_last_triggered"] = describeTimeSince(lastAlarmTriggeredAt);
    json["ap_ip"] = WiFi.softAPIP().toString();
    json["led_mode"] = currentState.ledMode;
    json["buzzer_enabled"] = currentState.buzzerEnabled;
    json["oled_mode"] = currentState.oledMode;
    json["time_last_sync"] = describeTimeSince(lastTimeSyncAt);

    String payload;
    serializeJson(json, payload);
    return payload;
}

String buildTelemetryHistoryJson()
{
    const size_t reserveBytes = 24 * telemetryHistoryCount + 256;
    DynamicJsonDocument doc(8192);
    JsonArray entries = doc.to<JsonArray>();

    size_t count = telemetryHistoryCount;
    for (size_t i = 0; i < count; ++i)
    {
        size_t idx = (telemetryHistoryIndex + TELEMETRY_HISTORY_SIZE - count + i) % TELEMETRY_HISTORY_SIZE;
        const TelemetrySample &sample = telemetryHistory[idx];
        if (!sample.valid)
        {
            continue;
        }

        JsonObject entry = entries.createNestedObject();
        entry["epoch"] = sample.epoch;
        entry["temperature"] = sample.temperature;
        entry["humidity"] = sample.humidity;
    }

    String payload;
    serializeJson(doc, payload);
    return payload;
}

void sendStateSnapshot()
{
    DynamicJsonDocument doc(512);
    doc["oled_mode"] = currentState.oledMode;
    doc["oled_custom_text"] = currentState.oledText;
    doc["led_mode"] = currentState.ledMode;
    doc["buzzer_enabled"] = currentState.buzzerEnabled;
    doc["alarm_enabled"] = currentState.alarmEnabled;
    doc["alarm_time"] = currentState.alarmTime;

    String payload;
    serializeJson(doc, payload);
    localServer.send(200, "application/json", payload);
}

bool loadJsonBody(DynamicJsonDocument &doc)
{
    if (!localServer.hasArg("plain"))
    {
        localServer.send(400, "text/plain", "Missing JSON payload");
        return false;
    }

    DeserializationError error = deserializeJson(doc, localServer.arg("plain"));
    if (error)
    {
        localServer.send(400, "text/plain", "Invalid JSON payload");
        return false;
    }

    return true;
}

void handleDisplayForm()
{
    DynamicJsonDocument doc(768);
    if (!loadJsonBody(doc))
    {
        return;
    }

    String requestedMode = String(doc["oled_mode"] | "");
    if (requestedMode.isEmpty())
    {
        requestedMode = String(doc["lcd_mode"] | "sensor"); // Backwards compatibility with legacy UI
    }

    if (requestedMode != "sensor" && requestedMode != "clock" && requestedMode != "custom")
    {
        requestedMode = "sensor";
    }

    String requestedText = String(doc["oled_custom_text"] | "");
    if (requestedText.isEmpty())
    {
        requestedText = String(doc["lcd_custom_text"] | "");
    }
    requestedText.trim();

    currentState.oledMode = requestedMode;
    currentState.oledText = requestedText;

    updateDisplays();

    localServer.send(200, "application/json", "{\"status\":\"ok\"}");
}

void handleAlarmForm()
{
    DynamicJsonDocument doc(256);
    if (!loadJsonBody(doc))
    {
        return;
    }

    currentState.alarmEnabled = doc["alarm_enabled"] | false;
    currentState.alarmTime = String(doc["alarm_time"] | "");

    updateDisplays();

    localServer.send(200, "application/json", "{\"status\":\"ok\"}");
}

void handleOutputsForm()
{
    DynamicJsonDocument doc(256);
    if (!loadJsonBody(doc))
    {
        return;
    }

    currentState.ledMode = String(doc["led_mode"] | "auto");
    currentState.buzzerEnabled = doc["buzzer_enabled"] | false;

    applyOutputs();

    localServer.send(200, "application/json", "{\"status\":\"ok\"}");
}

void updateDisplays()
{
    updateOledScene();
}

void updateOledScene()
{
    oled.clearDisplay();
    oled.setTextColor(SSD1306_WHITE);

    if (currentState.oledMode == "sensor")
    {
        oled.setTextSize(2);
        oled.setCursor(0, 0);
        oled.printf("%2.1fC\n", lastTemperature);
        oled.printf("%2.1f%%\n", lastHumidity);
    }
    else if (currentState.oledMode == "custom" && currentState.oledText.length() > 0)
    {
        oled.setTextSize(1);
        oled.setCursor(0, 0);
        oled.println(currentState.oledText);
    }
    else
    {
        oled.setTextSize(2);
        oled.setCursor(0, 0);
        oled.println(getCurrentClockString());
        oled.setTextSize(1);
        oled.setCursor(0, 40);
        oled.print("IP: ");
        oled.println(WiFi.localIP());
    }

    drawAlarmBadge();
    oled.display();
}

void drawAlarmBadge()
{
    if (!currentState.alarmEnabled)
    {
        return;
    }

    oled.fillRect(0, OLED_HEIGHT - 10, OLED_WIDTH, 10, SSD1306_BLACK);
    oled.setTextSize(1);
    oled.setCursor(0, OLED_HEIGHT - 9);
    oled.print("Alarm Set");
    if (currentState.alarmTime.length() == 5)
    {
        oled.print(" ");
        oled.print(currentState.alarmTime);
    }
}

void applyOutputs()
{
    bool ledOn = false;
    if (currentState.ledMode == "on")
    {
        ledOn = true;
    }
    else if (currentState.ledMode == "off")
    {
        ledOn = false;
    }
    else
    {
        if (!isnan(lastTemperature) && lastTemperature > TEMP_THRESHOLD_AUTO)
        {
            ledOn = true;
        }
        if (!isnan(lastHumidity) && lastHumidity > HUM_THRESHOLD_AUTO)
        {
            ledOn = true;
        }
    }

    digitalWrite(LED_PIN, ledOn ? HIGH : LOW);
}

void handleAlarm()
{
    if (!currentState.alarmEnabled)
    {
        alarmLatched = false;
        return;
    }

    if (shouldTriggerAlarm())
    {
        if (!alarmLatched)
        {
            triggerAlarmFeedback();
            lastAlarmTriggeredAt = millis();
            alarmLatched = true;
        }
    }
    else
    {
        alarmLatched = false;
    }
}

bool shouldTriggerAlarm()
{
    if (currentState.alarmTime.length() != 5)
    {
        return false;
    }

    DateTime now = rtc.now();
    char buffer[6];
    snprintf(buffer, sizeof(buffer), "%02d:%02d", now.hour(), now.minute());
    return currentState.alarmTime == String(buffer);
}

void triggerAlarmFeedback()
{
    if (!currentState.buzzerEnabled)
    {
        return;
    }

    Serial.println(F("[ALARM] Triggered"));
    unsigned long start = millis();
    while (millis() - start < 30000)
    {
        digitalWrite(BUZZER_PIN, HIGH);
        digitalWrite(LED_PIN, HIGH);
        delay(250);
        digitalWrite(BUZZER_PIN, LOW);
        digitalWrite(LED_PIN, LOW);
        delay(250);
    }
}

String getCurrentClockString()
{
    DateTime now = rtc.now();
    char buffer[9];
    snprintf(buffer, sizeof(buffer), "%02d:%02d:%02d", now.hour(), now.minute(), now.second());
    return String(buffer);
}

String describeTimeSince(unsigned long timestamp)
{
    if (timestamp == 0)
    {
        return String("Never");
    }

    unsigned long delta = millis() - timestamp;
    if (delta < 3000)
    {
        return String("Just now");
    }

    if (delta < 60000)
    {
        return String(delta / 1000) + "s ago";
    }

    if (delta < 3600000UL)
    {
        return String(delta / 60000UL) + "m ago";
    }

    return String(delta / 3600000UL) + "h ago";
}

void recordTelemetrySample(float temperature, float humidity)
{
    TelemetrySample &slot = telemetryHistory[telemetryHistoryIndex];
    slot.temperature = temperature;
    slot.humidity = humidity;
    slot.valid = true;

    DateTime now = rtc.now();
    if (now.year() < 2000)
    {
        slot.epoch = millis() / 1000UL;
    }
    else
    {
        slot.epoch = now.unixtime();
    }

    telemetryHistoryIndex = (telemetryHistoryIndex + 1) % TELEMETRY_HISTORY_SIZE;
    if (telemetryHistoryCount < TELEMETRY_HISTORY_SIZE)
    {
        telemetryHistoryCount++;
    }
}
