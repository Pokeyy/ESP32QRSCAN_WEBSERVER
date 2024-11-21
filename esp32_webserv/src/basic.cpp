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

ESP32QRCodeReader reader(CAMERA_MODEL_AI_THINKER);


void onQrCodeTask(void *pvParameters)
{
  struct QRCodeData qrCodeData;

  while (true)
  {
    
    if (frame_counter % 10 == 0 && reader.receiveQrCode(&qrCodeData, 100)) {
        frame_counter++;
    } else {
        frame_counter++;
        vTaskDelay(100 / portTICK_PERIOD_MS);
        continue;
    }
    
    {
      Serial.println("Found QRCode");
      if (qrCodeData.valid)
      {
        Serial.print("Payload: ");
        Serial.println((const char *)qrCodeData.payload);
      }
      else
      {
        Serial.print("Invalid: ");
        Serial.println((const char *)qrCodeData.payload);
      }
    }
    vTaskDelay(100 / portTICK_PERIOD_MS);
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
    
    //static int frame_counter = 0;
void setup()
{
  Serial.begin(115200);
  Serial.println();


  WiFi.begin(ssid, password);
  WiFi.setSleep(false);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");

  startCameraServer();

  Serial.print("Camera Ready! Use 'http://");
  Serial.print(WiFi.localIP());
  Serial.println("' to connect");

  reader.setup();

  Serial.println("Setup QRCode Reader");

  reader.beginOnCore(1);

  Serial.println("Begin on Core 1");

  xTaskCreate(onQrCodeTask, "onQrCode", 4 * 1024, NULL, 3, NULL);
  delay(200);
}

void loop()
{
  delay(10000);
}