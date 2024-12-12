#ifndef ESP32_QR_CODE_ARDUINO_H_
#define ESP32_QR_CODE_ARDUINO_H_

#include "Arduino.h"
#include "ESP32CameraPins.h"
#include "esp_camera.h"
//#include <Arduino.h>
#include "esp_http_server.h"
#include "esp_timer.h"
//#include "esp_camera.h"
#include "img_converters.h"
#include "fb_gfx.h"
#include "esp32-hal-ledc.h"
#include "sdkconfig.h"
#include "camera_index.h"

#ifndef QR_CODE_READER_STACK_SIZE
#define QR_CODE_READER_STACK_SIZE 40 * 1024
#endif

#ifndef QR_CODE_READER_TASK_PRIORITY
#define QR_CODE_READER_TASK_PRIORITY 5
#endif

extern SemaphoreHandle_t camera_mutex;

enum QRCodeReaderSetupErr
{
  SETUP_OK,
  SETUP_NO_PSRAM_ERROR,
  SETUP_CAMERA_INIT_ERROR,
};

/* This structure holds the decoded QR-code data */
struct QRCodeData
{
  bool valid;
  int dataType;
  uint8_t payload[1024];
  int payloadLen;
};

class ESP32QRCodeReader
{
private:
  TaskHandle_t qrCodeTaskHandler;
  CameraPins pins;
  framesize_t frameSize;

public:
  camera_config_t cameraConfig;
  QueueHandle_t qrCodeQueue;
  bool begun = false;
  bool debug = false;

  // Constructor
  ESP32QRCodeReader();
  ESP32QRCodeReader(CameraPins pins);
  ESP32QRCodeReader(CameraPins pins, framesize_t frameSize);
  ESP32QRCodeReader(framesize_t frameSize);
  ~ESP32QRCodeReader();

  // Setup camera
  QRCodeReaderSetupErr setup();

  void begin();
  void beginOnCore(BaseType_t core);
  bool receiveQrCode(struct QRCodeData *qrCodeData, long timeoutMs);
  void end();

  void setDebug(bool);
};

#endif