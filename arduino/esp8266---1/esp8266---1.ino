// ESP8266 1

#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <Keypad.h>

// ================= configuration =================

// WiFi Credentials
const char* ssid = "Mahdi";
const char* password = "mlj14021400";

// Raspberry Pi IP (Mosquitto Broker)
const char* mqtt_server = "10.95.51.100"; 
const int mqtt_port = 1883; 

// MQTT Topics
const char* topic_lamp_set = "greenhouse/main/lamp/set"; // Incoming site commands
const char* topic_main_status = "greenhouse/main/status"; // Outgoing main telemetry

// NodeMCU Relay Pin (D1 / GPIO 5)
const int LAMP_RELAY_PIN = 5; 

// Password Configurations
const String CORRECT_PASSWORD = "1234"; 
String inputPassword = "";              

// Keypad 3x4 Setup
const byte ROWS = 4; 
const byte COLS = 3; 

char keys[ROWS][COLS] = {
  {'1','2','3'},
  {'4','5','6'},
  {'7','8','9'},
  {'*','0','#'}
};

// Keypad NodeMCU Pins: D2, D3, D4, D5, D6, D7, D8
byte rowPins[ROWS] = {4, 0, 2, 14}; // Rows: D2, D3, D4, D5
byte colPins[COLS] = {12, 13, 15};  // Cols: D6, D7, D8

Keypad keypad = Keypad( makeKeymap(keys), rowPins, colPins, ROWS, COLS );

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

// Function to sync the physical lamp state with the Web dashboard
void publishLampStatus() {
  JsonDocument doc;
  
  // Read current physical state and map to "ON"/"OFF" string required by JS dashboard
  int currentState = digitalRead(LAMP_RELAY_PIN);
  doc["lamp"] = (currentState == HIGH) ? "ON" : "OFF"; 

  char buffer[128];
  serializeJson(doc, buffer);
  
  // Publish to specific lamp state topic to avoid UI chart zeroing
  client.publish("greenhouse/main/lamp", buffer); 
}

// Callback for processing incoming control commands from the dashboard
void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");

  String messageBuffer = "";
  for (unsigned int i = 0; i < length; i++) {
    messageBuffer += (char)payload[i];
  }
  Serial.println(messageBuffer);

  if (String(topic) == topic_lamp_set) {
    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, messageBuffer);

    if (!error) {
      String lampState = doc["state"]; // Checks {"state": "ON"} or "OFF"
      if (lampState == "ON") {
        digitalWrite(LAMP_RELAY_PIN, HIGH);
        Serial.println("Lamp turned ON from Site");
      } else if (lampState == "OFF") {
        digitalWrite(LAMP_RELAY_PIN, LOW);
        Serial.println("Lamp turned OFF from Site");
      }
    }
  }
}

void reconnect() {
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    String clientId = "ESP8266-LampControl-";
    clientId += String(random(0xffff), HEX);
    
    if (client.connect(clientId.c_str())) {
      Serial.println("connected");
      client.subscribe(topic_lamp_set); 
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
  
  pinMode(LAMP_RELAY_PIN, OUTPUT);
  digitalWrite(LAMP_RELAY_PIN, LOW); // Default state: OFF

  setup_wifi();
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  // --- Keypad Management ---
  char key = keypad.getKey();
  
  if (key) {
    Serial.print("Key Pressed: ");
    Serial.println(key);
    
    if (key == '#') {
      // '#' key acts as ENTER / Verification
      if (inputPassword == CORRECT_PASSWORD) {
        Serial.println("Password Correct! Toggling Lamp...");
        
        // Toggle the current relay physical state
        int currentState = digitalRead(LAMP_RELAY_PIN);
        digitalWrite(LAMP_RELAY_PIN, !currentState);
        
        // Send state feedback up to the broker and dashboard
        publishLampStatus();
      } else {
        Serial.println("Incorrect Password!");
      }
      inputPassword = ""; // Clear password buffer after processing
    } 
    else if (key == '*') {
      // '*' key acts as BACKSPACE / CLEAR
      inputPassword = "";
      Serial.println("Password Cleared.");
    } 
    else {
      // Append characters to current active entry string
      inputPassword += key;
    }
  }
}