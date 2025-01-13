#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include <ESP8266HTTPClient.h>

// WiFi Credentials
const char* ssid = "TECNO CAMON 12 Air";
const char* password = "afms83/406";

// API Base URL
const char* apiBaseUrl = "https://light-control-app.onrender.com"; // Replace with your Render API URL

// Pin Configuration
const int lightPin = 14;

// Timer Configuration
unsigned long lastStatusUpdate = 0;
unsigned long lastStateCheck = 0;
const unsigned long statusUpdateInterval = 10000; // 10 seconds
const unsigned long stateCheckInterval = 5000;   // 5 seconds

// Current Light State
String currentState = "off";

WiFiClientSecure client; // Use WiFiClientSecure for HTTPS

void setup() {
  // Initialize Serial and Pin
  Serial.begin(115200);
  pinMode(lightPin, OUTPUT);
  digitalWrite(lightPin, LOW); // Start with the light off

  // Connect to Wi-Fi
  connectToWiFi();

  // Notify backend that ESP8266 is online
  notifyStatus(true);
}

void loop() {
  unsigned long currentMillis = millis();

  // Update ESP8266 status periodically
  if (currentMillis - lastStatusUpdate >= statusUpdateInterval) {
    notifyStatus(true);
    lastStatusUpdate = currentMillis;
  }

  // Check the light state from the backend periodically
  if (currentMillis - lastStateCheck >= stateCheckInterval) {
    checkLightState();
    lastStateCheck = currentMillis;
  }
}

void connectToWiFi() {
  Serial.print("Connecting to Wi-Fi");
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }
  Serial.println("\nConnected to Wi-Fi");
}

void notifyStatus(bool online) {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    String url = String(apiBaseUrl) + "/status";
    client.setInsecure(); // Allow insecure connections (not recommended for production)
    http.begin(client, url); // Use WiFiClientSecure for HTTPS
    http.addHeader("Content-Type", "application/json");

    String payload = "{\"online\":" + String(online ? "true" : "false") + "}";
    int httpResponseCode = http.POST(payload);

    if (httpResponseCode > 0) {
      Serial.println("Status update sent: " + payload);
    } else {
      Serial.println("Failed to send status update");
    }

    http.end();
  }
}

void checkLightState() {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    String url = String(apiBaseUrl) + "/command";
    client.setInsecure(); // Allow insecure connections (not recommended for production)
    http.begin(client, url); // Use WiFiClientSecure for HTTPS

    int httpResponseCode = http.GET();
    if (httpResponseCode > 0) {
      String response = http.getString();
      Serial.println("Light state response: " + response);

      // Parse the response
      int stateIndex = response.indexOf("lightState");
      if (stateIndex != -1) {
        String newState = response.substring(response.indexOf(":", stateIndex) + 2, response.indexOf("\"", stateIndex + 14));

        // Update light if the state has changed
        if (newState != currentState) {
          currentState = newState;
          digitalWrite(lightPin, currentState == "on" ? HIGH : LOW);
          Serial.println("Light state updated to: " + currentState);
        }
      }
    } else {
      Serial.println("Failed to fetch light state");
    }

    http.end();
  }
}
