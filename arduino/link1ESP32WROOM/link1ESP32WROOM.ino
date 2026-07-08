#include <ESP8266WiFi.h> // تغییر کتابخانه متناسب با ESP8266
#include <PubSubClient.h>
#include <ArduinoJson.h>

// ================= configuration =================

// مشخصات وای‌فای شما
const char* ssid = "Mahdi";
const char* password = "mlj14021400";

// آی‌پی رزبری پای (بروکر ماسکویتو) - آی‌پی رزبری خود را اینجا جایگزین کنید
const char* mqtt_server = "10.95.51.100"; 
const int mqtt_port = 1883; 

// تاپیک‌های MQTT مربوط به خط ۱
const char* topic_status = "greenhouse/line1/status";
const char* topic_pump_control = "greenhouse/line1/pump/set";

// تعریف پین‌ها روی ESP8266 (NodeMCU)
const int SOIL_PIN = A0;       // تنها پین آنالوگ در ESP8266
const int PUMP_RELAY_PIN = 5;  // پین دیجیتال D1 یا همان GPIO 5 برای رله پمپ

// مقادیر کالیبراسیون سنسور خاک برای ADC ده بیتی ESP8266 (بازه 0 تا 1023)
// مقادیر زیر را بر اساس تست خود در حالت کاملا خشک و داخل آب اصلاح کنید
const int DRY_VALUE = 850;   
const int WET_VALUE = 250;   

unsigned long lastMsg = 0;
const long interval = 5000; // ارسال داده‌ها هر ۵ ثانیه یک‌بار

// =================================================

WiFiClient espClient;
PubSubClient client(espClient);

void setup_wifi() {
  delay(10);
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\nWiFi connected");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}

// تابع دریافت فرمان برای روشن/خاموش کردن پمپ از طریق سایت
void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");

  String messageBuffer = "";
  for (unsigned int i = 0; i < length; i++) { // اصلاح نوع داده i به unsigned int
    messageBuffer += (char)payload[i];
  }
  Serial.println(messageBuffer);

  if (String(topic) == topic_pump_control) {
    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, messageBuffer);

    if (!error) {
      String pumpState = doc["state"]; // دریافت وضعیت {"state": "ON"} یا OFF
      if (pumpState == "ON") {
        digitalWrite(PUMP_RELAY_PIN, HIGH); // روشن کردن رله پمپ
        Serial.println("Pump turned ON");
      } else if (pumpState == "OFF") {
        digitalWrite(PUMP_RELAY_PIN, LOW);  // خاموش کردن رله پمپ
        Serial.println("Pump turned OFF");
      }
    }
  }
}

void reconnect() {
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    String clientId = "ESP8266-Line1-";
    clientId += String(random(0xffff), HEX);
    
    if (client.connect(clientId.c_str())) {
      Serial.println("connected");
      client.subscribe(topic_pump_control); // سابسکرایب روی تاپیک پمپ
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}

void setup() {
  Serial.begin(115200);
  
  pinMode(PUMP_RELAY_PIN, OUTPUT);
  digitalWrite(PUMP_RELAY_PIN, LOW); // حالت پیش‌فرض پمپ خاموش است

  // در ESP8266 پین A0 به صورت پیش‌فرض ورودی آنالوگ است و نیاز مبرمی به pinMode ندارد، اما نوشتن آن مشکلی ایجاد نمی‌کند.
  pinMode(SOIL_PIN, INPUT);

  setup_wifi();
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  unsigned long now = millis();
  if (now - lastMsg > interval) {
    lastMsg = now;

    // خواندن مقدار خام آنالوگ از سنسور رطوبت خاک (بازه 0 تا 1023)
    int rawAnalog = analogRead(SOIL_PIN);
    
    // تبدیل مقدار خام به درصد (۰ تا ۱۰۰ درصد) 
    int soilHumidityPercentage = map(rawAnalog, DRY_VALUE, WET_VALUE, 0, 100);
    
    // محدود کردن مقدار بین ۰ تا ۱۰۰ جهت جلوگیری از اعداد خارج از محدوده
    if(soilHumidityPercentage > 100) soilHumidityPercentage = 100;
    if(soilHumidityPercentage < 0) soilHumidityPercentage = 0;

    // خواندن وضعیت فعلی رله پمپ
    String pumpStatus = (digitalRead(PUMP_RELAY_PIN) == HIGH) ? "ACTIVE" : "IDLE";

    // ساخت پکت JSON کاملاً هماهنگ با فرانت‌اند سایت شما
    JsonDocument doc;
    doc["soilhemu"] = soilHumidityPercentage;
    doc["pump"] = pumpStatus;

    char buffer[256];
    serializeJson(doc, buffer);

    // ارسال داده‌ها به تاپیک خط ۱ در رزبری پای
    client.publish(topic_status, buffer);
    
    Serial.print("Published to Line 1: ");
    Serial.println(buffer);
  }
}
