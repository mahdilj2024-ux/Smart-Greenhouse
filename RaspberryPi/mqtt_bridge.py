import paho.mqtt.client as mqtt
import requests
import json

# --- Network Configurations ---
MQTT_BROKER = "localhost"          # Running on the same Raspberry Pi
LAPTOP_IP = "10.95.51.7"           # Laptop IP running Laragon
LARAGON_URL = f"http://{LAPTOP_IP}/greenhouse/api/save_data.php" 

# Target MQTT Topics
TOPICS = [
    ("greenhouse/main/status", 0),
    ("greenhouse/main/lamp", 0)
]

def on_connect(client, userdata, flags, rc, properties=None):
    if rc == 0:
        print("Success: Connected to Mosquitto Broker.")
        # Subscribe to all required topics
        client.subscribe(TOPICS)
    else:
        print(f"Error: Connection failed with code {rc}")

def on_message(client, userdata, msg):
    try:
        # Decode and parse incoming JSON payload
        payload = msg.payload.decode("utf-8")
        data = json.loads(payload)
        data["topic"] = msg.topic  # Include topic to differentiate in PHP backend
        
        print(f"Received data from [{msg.topic}]: {payload}")
        
        # Forward the JSON packet to Laragon API
        headers = {'Content-Type': 'application/json'}
        response = requests.post(LARAGON_URL, json=data, headers=headers, timeout=5)
        
        if response.status_code == 200:
            print("Status: Data forwarded and saved to Laragon successfully.")
        else:
            print(f"Warning: Laragon returned status code {response.status_code}")
            
    except Exception as e:
        print(f"Error: Failed to process or forward data: {e}")

# Initialize MQTT Client (Compatible with Paho MQTT v1.x and v2.x)
client = mqtt.Client(callback_api_version=mqtt.CallbackAPIVersion.VERSION2)
client.on_connect = on_connect
client.on_message = on_message

print("Connecting to Mosquitto Broker on Raspberry Pi...")
client.connect(MQTT_BROKER, 1883, 60)

# Start the infinite network loop
client.loop_forever()