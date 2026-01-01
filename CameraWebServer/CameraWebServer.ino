#include "esp_camera.h"
#include <WiFi.h>

// ===================
// CAMERA MODEL
// ===================
#define CAMERA_MODEL_AI_THINKER
#include "camera_pins.h"



// ===================
// AI THINKER ESP32-CAM PINS
// ===================
#define PWDN_GPIO_NUM     32
#define RESET_GPIO_NUM    -1
#define XCLK_GPIO_NUM      0
#define SIOD_GPIO_NUM     26
#define SIOC_GPIO_NUM     27

#define Y9_GPIO_NUM       35
#define Y8_GPIO_NUM       34
#define Y7_GPIO_NUM       39
#define Y6_GPIO_NUM       36
#define Y5_GPIO_NUM       21
#define Y4_GPIO_NUM       19
#define Y3_GPIO_NUM       18
#define Y2_GPIO_NUM        5
#define VSYNC_GPIO_NUM    25
#define HREF_GPIO_NUM     23
#define PCLK_GPIO_NUM     22

// ===========================
// WIFI CREDENTIALS
// ===========================
const char* ssid = "ISHANT_BHANDARI 5770";
const char* password = "18090501";

void startCameraServer();

void setup() {
  Serial.begin(115200);
  Serial.setDebugOutput(false);
  Serial.println();

  // ===================
  // CAMERA CONFIG
  // ===================
  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer   = LEDC_TIMER_0;

  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;

  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href  = HREF_GPIO_NUM;
  config.pin_sccb_sda = SIOD_GPIO_NUM;
  config.pin_sccb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn  = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;

  config.xclk_freq_hz = 20000000;

  // ===== OV3660 SAFE SETTINGS =====
  config.pixel_format = PIXFORMAT_JPEG;
  config.frame_size   = FRAMESIZE_QVGA;     // 320x240 (stable)
  config.jpeg_quality = 20;                 // lower load
  config.fb_count     = 1;                  // SINGLE buffer only
  config.grab_mode    = CAMERA_GRAB_WHEN_EMPTY;
  config.fb_location  = CAMERA_FB_IN_PSRAM;

  // ===================
  // INIT CAMERA
  // ===================
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Camera init failed: 0x%x\n", err);
    return;
  }

  // ===================
  // SENSOR TUNING (OV3660)
  // ===================
  sensor_t * s = esp_camera_sensor_get();
  if (s->id.PID == OV3660_PID) {
    s->set_vflip(s, 1);
    s->set_brightness(s, 1);
    s->set_saturation(s, -2);
    s->set_framesize(s, FRAMESIZE_QVGA);
  }

  // ===================
  // DISABLE FLASH (GPIO4)
  // ===================
  pinMode(4, OUTPUT);
  digitalWrite(4, LOW);   // FORCE FLASH OFF

  // ===================
  // WIFI
  // ===================
  WiFi.begin(ssid, password);
  WiFi.setSleep(false);

  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println();
  Serial.println("WiFi connected");

  // ===================
  // START SERVER
  // ===================
  startCameraServer();

  Serial.print("Camera Ready! Use http://");
  Serial.print(WiFi.localIP());
  Serial.println(" to connect");
}

void loop() {
  delay(10000);
}
