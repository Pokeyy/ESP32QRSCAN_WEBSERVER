#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <ESP32QRCodeReader.h>
#include <HTTPClient.h>

// Wi-Fi credentials
const char* ssid = "ASUS_2.4G";
const char* password = "111627284450";

// Web server on port 80
WebServer server(80);

// QR Code reader setup
ESP32QRCodeReader qrReader(CAMERA_MODEL_AI_THINKER);
QRCodeData qrCodeData;  // Store QR code data

// Wi-Fi connection check
bool connectWifi() {
  if (WiFi.status() == WL_CONNECTED) {
    return true;
  }

  WiFi.begin(ssid, password);
  int maxRetries = 10;
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    maxRetries--;
    if (maxRetries <= 0) {
      return false;
    }
  }
  Serial.println("");
  Serial.println("WiFi connected");
  return true;
}

// Send QR code data to server
void sendToServer(String qrCode) {
  HTTPClient http;
  String serverURL = "http://192.168.2.82/sendData";  // Replace <ESP32_IP> with your actual ESP32 IP address
  String payload = "code=" + qrCode;

  http.begin(serverURL);  // Initialize HTTP client
  http.addHeader("Content-Type", "application/x-www-form-urlencoded");  // Form data header
  int httpCode = http.POST(payload);  // Send POST request with QR code data
  
  if (httpCode > 0) {
    String response = http.getString();
    Serial.println("Response from server: " + response);
  } else {
    Serial.println("Error sending POST request");
  }

  http.end();  // Close the connection
}

void setup() {
  Serial.begin(115200);
  pinMode(2, OUTPUT);  // Optional: use the built-in LED for status indication

  // Connect to Wi-Fi
  if (!connectWifi()) {
    Serial.println("Failed to connect to WiFi");
    return;
  }

  // Print ESP32 IP address
  Serial.println("ESP32 IP address: ");
  Serial.println(WiFi.localIP());

  // Set up QR code reader
  qrReader.setup();
  qrReader.setDebug(true);
  qrReader.begin();
  Serial.println("QR Code Reader Initialized");

  // Set up web server routes
  server.on("/getData", HTTP_GET, []() {
    server.send(200, "text/plain", "Hello from ESP32!");
  });

  server.on("/sendData", HTTP_POST, []() {
    String receivedData = server.arg("code");  // Get the QR code data sent in the POST body
    Serial.println("Received code: " + receivedData);

    if (receivedData == "VALID_QR_CODE") {  // Check if the QR code is valid
      server.send(200, "text/plain", "Valid QR code, access granted!");
    } else {
      server.send(200, "text/plain", "Invalid QR code, access denied!");
    }
  });

  // Start the web server
  server.begin();
  Serial.println("Web server started.");
}

void loop() {
  server.handleClient();  // Handle any incoming HTTP requests

  // Check if a QR code is received
  if (qrReader.receiveQrCode(&qrCodeData, 100)) {  // Wait 100ms for QR code
    Serial.println("QR Code Found");

    if (qrCodeData.valid) {
      String qrCode = String((const char*)qrCodeData.payload);
      Serial.println("QR Code payload: " + qrCode);
      
      // Send QR code data to server
      sendToServer(qrCode);
    } else {
      Serial.println("Invalid QR Code");
    }
  }

  delay(100);  // Delay before checking again
}
