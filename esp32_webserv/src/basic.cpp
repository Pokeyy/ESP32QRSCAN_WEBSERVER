#include <Arduino.h>
#include <ESP32QRCodeReader.h>
#include "esp_camera.h"
#include <WiFi.h>
#include <esp_wifi.h>
#include "camera_pins.h"

//#define CAMERA_MODEL_AI_THINKER
SemaphoreHandle_t camera_mutex = xSemaphoreCreateMutex();  // Define the mutex here
const char *ssid = "ASUS_2.4G";
const char *password = "111627284450";
static int frame_counter = 0; 

//ESP32QRCodeReader reader(CAMERA_MODEL_AI_THINKER);
ESP32QRCodeReader reader(CAMERA_MODEL_XIAO_ESP32S3);

void triggerServo()
{

}

void onQrCodeTask(void *pvParameters)
{
  struct QRCodeData qrCodeData;
  static String lastPayload = "";
  static int repeatCount = 0;

  while (true)
  {
    if (reader.receiveQrCode(&qrCodeData, 100)) {
      if (qrCodeData.valid) {
        String current = String((const char *)qrCodeData.payload);

        if (current == lastPayload) {
          repeatCount++;
        } else {
          repeatCount = 1;
          lastPayload = current;
        }

        Serial.printf("QR: %s (count: %d)\n", current.c_str(), repeatCount);

        // Example: trigger after 5 consistent reads
        if (repeatCount == 10) {
          repeatCount = 0;
          Serial.println("Dropping!");
          Serial.flush();
          delay(20);
          // triggerServo(); // need servo logic
        }
      }
    }

    vTaskDelay(50 / portTICK_PERIOD_MS); // Smooth scheduling
  }
}

void startCameraServer();
void setupLedFlash(int pin);

void readMacAddress(){
  uint8_t baseMac[6];
  esp_err_t ret = esp_wifi_get_mac(WIFI_IF_STA, baseMac);
  if (ret == ESP_OK) {
    Serial.printf("%02x:%02x:%02x:%02x:%02x:%02x\n",
                  baseMac[0], baseMac[1], baseMac[2],
                  baseMac[3], baseMac[4], baseMac[5]);
  } 
  else {
    Serial.println("Failed to read MAC address");
  }
}


  //
  //SemaphoreHandle_t camera_mutex = xSemaphoreCreateMutex();
    
void setup()
{
  esp_log_level_set("cam_hal", ESP_LOG_NONE);
  if (!psramFound()) {
    Serial.println("PSRAM not found! Reduce memory usage.");
  }
  Serial.begin(115200);
  Serial.println();

  if (!psramFound()) {
    Serial.println("PSRAM not found! Reduce memory usage.");
  }

  WiFi.begin(ssid, password);
  WiFi.setSleep(false);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");

  startCameraServer();

  //Serial.printf("Free heap size after camera setup: %u\n", esp_get_free_heap_size());

  Serial.print("Camera Ready! Use 'http://");
  Serial.print(WiFi.localIP());
  Serial.println("' to connect");

  reader.setup();

  Serial.println("Setup QRCode Reader");

  reader.beginOnCore(1);

  Serial.println("Begin on Core 1");

  xTaskCreate(onQrCodeTask, "onQrCode", 8 * 1024, NULL, 3, NULL);
  delay(200);
}

void loop()
{
  delay(10000);
}