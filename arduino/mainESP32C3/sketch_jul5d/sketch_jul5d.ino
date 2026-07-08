#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <Wire.h>
#include <DHT.h>
#include <Adafruit_BMP280.h>
#include <Adafruit_INA219.h>
#include <TM1637Display.h>
#include <time.h>

// ================= configuration =================
const char* ssid = "Mahdi";
const char* password = "mlj14021400";
const char* mqtt_server = "10.95.51.100"; 
const int mqtt_port = 1883; 

const char* topic_status = "greenhouse/main/status";
const char* topic_fan_control = "greenhouse/main/fan/set";

// تعریف پین‌ها
#define DHTPIN 4           
#define DHTTYPE DHT22      
const int FAN_RELAY_PIN = 2; 

// پایه‌های I2C برای بارومتر و سنسور جریان (مشترک)
#define I2C_SDA 8
#define I2C_SCL 9

// پین‌های ماژول سون‌سگمنت TM1637
#define TM_CLK 5
#define TM_DIO 6

// تنظیمات ساعت سرور شبکه (NTP)
const char* ntpServer = "pool.ntp.org";
const long  gmtOffset_sec = 12600; // اختلاف ساعت ایران با گرینویچ (+3:30 = 12600 ثانیه)
const int   daylightOffset_sec = 0; // در صورت تغییر ساعت تابستانی تنظیم شود

// =================================================

WiFiClient espClient;
PubSubClient client(espClient);

DHT dht(DHTPIN, DHTTYPE);
Adafruit_BMP280 bmp; 
Adafruit_INA219 ina219; // شیء سنسور جریان
TM1637Display display(TM_CLK, TM_DIO); // شیء سون‌سگمنت

unsigned long lastMsg = 0;
const long interval = 5000; 
bool dots = true; // برای چشمک زدن دونقطه (:) ساعت

void setup_wifi() {
  delay(10);
  Serial.println();
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected");
  
  // پیکربندی و دریافت زمان واقعی از اینترنت
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
}

void callback(char* topic, byte* payload, unsigned int length) {
  String messageBuffer = "";
  for (int i = 0; i < length; i++) messageBuffer += (char)payload[i];
  
  if (String(topic) == topic_fan_control) {
    JsonDocument doc;
    deserializeJson(doc, messageBuffer);
    String fanState = doc["state"];
    if (fanState == "ON") digitalWrite(FAN_RELAY_PIN, HIGH);
    else if (fanState == "OFF") digitalWrite(FAN_RELAY_PIN, LOW);
  }
}

void reconnect() {
  while (!client.connected()) {
    String clientId = "ESP32C3-Main-";
    clientId += String(random(0xffff), HEX);
    if (client.connect(clientId.c_str())) {
      client.subscribe(topic_fan_control);
    } else {
      delay(5000);
    }
  }
}

void setup() {
  Serial.begin(115200);
  pinMode(FAN_RELAY_PIN, OUTPUT);
  digitalWrite(FAN_RELAY_PIN, LOW); 

  dht.begin();
  Wire.begin(I2C_SDA, I2C_SCL);
  
  // راه‌اندازی سنسور فشار
  bmp.begin(0x76);
  
  // راه‌اندازی سنسور جریان INA219
  if (!ina219.begin()) {
    Serial.println("Failed to find INA219 chip");
  }

  // تنظیم روشنایی سون‌سگمنت (از 0 تا 7)
  display.setBrightness(5);

  setup_wifi();
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);
}

void loop() {
  if (!client.connected()) reconnect();
  client.loop();

  // --- مدیریت و نمایش ساعت روی سون سگمنت ---
  struct tm timeinfo;
  if (getLocalTime(&timeinfo)) {
    int displayTime = (timeinfo.tm_hour * 100) + timeinfo.tm_min;
    
    // چشمک زدن دو نقطه وسط ساعت در هر ثانیه
    dots = (millis() / 1000) % 2 == 0; 
    display.showNumberDecEx(displayTime, (dots ? 0b01000000 : 0), true);
  }

  // --- ارسال داده‌ها به سایت ---
  unsigned long now = millis();
  if (now - lastMsg > interval) {
    lastMsg = now;

    float hemu = dht.readHumidity();
    float temp = dht.readTemperature();
    float press = bmp.readPressure() / 100.0F;

    // خواندن جریان بر حسب میلی‌آمپر و تبدیل آن به آمپر (A)
    float current_mA = ina219.getCurrent_mA();
    float current_A = current_mA / 1000.0F; 
    if (current_A < 0) current_A = 0; // حذف نویزهای احتمالی منفی

    if (isnan(hemu) || isnan(temp)) { temp = 25.5; hemu = 55.0; }
    if (isnan(press) || press == 0) press = 1013.0;

    String fanStatus = (digitalRead(FAN_RELAY_PIN) == HIGH) ? "ON" : "OFF";

    JsonDocument doc;
    doc["temp"] = String(temp, 1);   
    doc["hemu"] = String(hemu, 0);   
    doc["press"] = String(press, 0); 
    doc["fan"] = fanStatus;
    doc["current"] = String(current_A, 3); // ارسال جریان مصرفی با ۳ رقم اعشار دقیق

    char buffer[256];
    serializeJson(doc, buffer);
    client.publish(topic_status, buffer);
    
    Serial.print("Published payload: ");
    Serial.println(buffer);
  }
}
