#include <WiFi.h>
#include <PubSubClient.h>

// Pin Definitions
#define LDR_PIN 34         // LDR sensor connected to pin 34 (analog input)
#define RELAY_PIN 23       // Relay connected to pin 23 (controls streetlight)
#define LED_PIN1 25        // LED1 for streetlight simulation
#define LED_PIN2 26        // LED2 for streetlight simulation
#define LED_PIN3 27        // LED3 for streetlight simulation

// WiFi Settings
const char* ssid = "Wokwi-GUEST";   // Your WiFi SSID
const char* password = ""; // Your WiFi password

// ThingsBoard Settings
const char* mqttServer = "demo.thingsboard.io"; // ThingsBoard server
const int mqttPort = 1883;                      // MQTT port
const char* accessToken = "wx7qhfe1rtdgd3b6hf8z";  // Access token for the device

// Create WiFi and MQTT clients
WiFiClient wifiClient;
PubSubClient client(wifiClient);

void setup() {
  Serial.begin(115200);
  pinMode(RELAY_PIN, OUTPUT);  // Set relay pin as output to control light
  pinMode(LDR_PIN, INPUT);     // Set LDR pin as input
  pinMode(LED_PIN1, OUTPUT);   // Set LED1 as output
  pinMode(LED_PIN2, OUTPUT);   // Set LED2 as output
  pinMode(LED_PIN3, OUTPUT);   // Set LED3 as output
  
  // Connect to WiFi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi");

  // Connect to ThingsBoard server
  client.setServer(mqttServer, mqttPort);
  while (!client.connected()) {
    Serial.print("Connecting to ThingsBoard...");
    if (client.connect("ESP32_Device", accessToken, NULL)) {
      Serial.println("connected");
    } else {
      Serial.print("failed with state ");
      Serial.print(client.state());
      delay(2000);
    }
  }
}

void loop() {
  if (!client.connected()) {
    // Reconnect if MQTT connection is lost
    while (!client.connected()) {
      Serial.print("Reconnecting to ThingsBoard...");
      if (client.connect("ESP32_Device", accessToken, NULL)) {
        Serial.println("reconnected");
      } else {
        Serial.print("failed with state ");
        Serial.print(client.state());
        delay(2000);
      }
    }
  }
  client.loop();

  // Read LDR value
  int ldrValue = analogRead(LDR_PIN);  // LDR value (light intensity)

  // Control light based on LDR
  if (ldrValue < 500) {
    digitalWrite(RELAY_PIN, HIGH);  // Turn streetlight ON
    digitalWrite(LED_PIN1, HIGH);
    digitalWrite(LED_PIN2, HIGH);
    digitalWrite(LED_PIN3, HIGH);
  } else {
    digitalWrite(RELAY_PIN, LOW);   // Turn streetlight OFF
    digitalWrite(LED_PIN1, LOW);
    digitalWrite(LED_PIN2, LOW);
    digitalWrite(LED_PIN3, LOW);
  }

  // Prepare JSON payload for ThingsBoard
  String payload = "{";
  payload += "\"ldrValue\":" + String(ldrValue) + ",";
  payload += "\"lightStatus\":" + String(digitalRead(RELAY_PIN));
  payload += "}";

  // Publish data to ThingsBoard
  Serial.println("Sending data to ThingsBoard: " + payload);
  client.publish("v1/devices/me/telemetry", payload.c_str());

  // Wait for 5 seconds before sending data again
  delay(5000);
}