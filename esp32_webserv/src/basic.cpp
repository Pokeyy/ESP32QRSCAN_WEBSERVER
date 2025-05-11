#include <Arduino.h>
#include <ESP32QRCodeReader.h>
#include "esp_camera.h"
#include <WiFi.h>
#include <esp_wifi.h>
#include "camera_pins.h"
#include <ESP32Servo.h>

//#define CAMERA_MODEL_AI_THINKER
SemaphoreHandle_t camera_mutex = xSemaphoreCreateMutex();  // Define the mutex here
const char *ssid = "ASUS_2.4G";
const char *password = "111627284450";
static int frame_counter = 0; 


//ESP32QRCodeReader reader(CAMERA_MODEL_AI_THINKER);
ESP32QRCodeReader reader(CAMERA_MODEL_XIAO_ESP32S3);

Servo myServo;
const int SERVO_PIN = 1;
const int VALID_LED = 2;
const int INVALID_LED = 3;
const int SERVO_CLOSED = 7;
const int SERVO_OPEN = 78;
int servoPos = 0;

void triggerServo()
{
  digitalWrite(VALID_LED, HIGH);
  myServo.write(SERVO_OPEN);
  delay(2000);  // Hold the LED on for 2 seconds after drop
  digitalWrite(VALID_LED, LOW);
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
          digitalWrite(VALID_LED, HIGH);
          repeatCount++;
          delay(10);
          digitalWrite(VALID_LED, LOW);
        } else {
          repeatCount = 1;
          lastPayload = current;
        }

        Serial.printf("QR: %s (count: %d)\n", current.c_str(), repeatCount);

        if(repeatCount == 10) {
          if(current == "https://www.cpp.edu/") {
            triggerServo();
          }
          else {
            digitalWrite(INVALID_LED, HIGH);
            delay(500);
            digitalWrite(INVALID_LED, LOW);
            myServo.write(SERVO_CLOSED);
          }
          repeatCount = 0;
        }
        // if (repeatCount == 10 && current == "https://www.cpp.edu/") {
        //   triggerServo();
        //   repeatCount = 0;
        //   // Serial.println("Valid QR Code - Dropping!");
        //   // Serial.flush();
        //   delay(100);
        //   digitalWrite(2, LOW);
        //   // triggerServo(); // need servo logic
        // }
        // else if(repeatCount == 10) {
        //   digitalWrite(INVALID_LED, HIGH);
        //   //Serial.println("Invalid QR Code.");
        //   repeatCount = 0;
        //   myServo.write(SERVO_CLOSED);
        //   delay(1000);
        //   digitalWrite(INVALID_LED, LOW);
        // }
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
    
void setup()
{
  esp_log_level_set("*", ESP_LOG_WARN);
  if (!psramFound()) {
    Serial.println("PSRAM not found! Reduce memory usage.");
  }
  Serial.begin(115200); // 115200 default
  delay(1000);
  pinMode(2, OUTPUT);
  pinMode(3, OUTPUT);
  myServo.attach(SERVO_PIN);        // GPIO1
  myServo.write(SERVO_CLOSED);
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
  sensor_t *s = esp_camera_sensor_get();
  if (s) {
    s->set_brightness(s, 1);              // match UI or set your own default
    s->set_contrast(s, 2);
    s->set_saturation(s, 0);
    s->set_gainceiling(s, (gainceiling_t)6);
    s->set_vflip(s, 1);                   // for physical orientation
    //s->set_framesize(s, FRAMESIZE_QVGA);  // Only works before init or if using JPEG
  }

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