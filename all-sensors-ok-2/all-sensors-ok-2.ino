#include <WiFi.h>
#include <WebServer.h>
#include <Wire.h>
#include <MPU6050.h>
#include <VL53L1X.h>
#include <TinyGPS++.h>
#include <driver/i2s.h>
#include <math.h>

/* ================= WIFI ================= */
const char* ssid = "ISHANT_BHANDARI 5770";
const char* password = "18090501";

/* ================= SERVER ================= */
WebServer server(80);

/* ================= SENSORS ================= */
MPU6050 mpu;
VL53L1X vl53;
TinyGPSPlus gps;

/* ================= I2S MIC ================= */
#define I2S_WS   25
#define I2S_SCK  26
#define I2S_SD   32
#define I2S_PORT I2S_NUM_0

#define SAMPLE_RATE 16000
#define BUFFER_LEN 256
#define NOISE_THRESHOLD 800

int32_t samples[BUFFER_LEN];

/* ================= MADGWICK ================= */
float q0 = 1, q1 = 0, q2 = 0, q3 = 0;
float beta = 0.08;  // conservative for stability
uint32_t lastIMUTime = 0;

/* ================= STATE ================= */
struct {
  float ax, ay, az;
  float gx, gy, gz;
  float pitch, roll, yaw;
  uint16_t altitude_cm;
  double lat, lon;
  float speed;
  bool gps_fix;
  float audio_rms;
  bool audio_spike;
  uint32_t uptime_ms;
  uint32_t heap_free;
} state;

/* ================= MADGWICK UPDATE ================= */
void madgwickUpdate(float gx, float gy, float gz,
                    float ax, float ay, float az,
                    float dt) {

  float recipNorm;
  float s0, s1, s2, s3;
  float qDot1, qDot2, qDot3, qDot4;

  recipNorm = sqrt(ax * ax + ay * ay + az * az);
  if (recipNorm == 0) return;
  recipNorm = 1.0f / recipNorm;
  ax *= recipNorm;
  ay *= recipNorm;
  az *= recipNorm;

  qDot1 = 0.5f * (-q1 * gx - q2 * gy - q3 * gz);
  qDot2 = 0.5f * ( q0 * gx + q2 * gz - q3 * gy);
  qDot3 = 0.5f * ( q0 * gy - q1 * gz + q3 * gx);
  qDot4 = 0.5f * ( q0 * gz + q1 * gy - q2 * gx);

  s0 = 4*q0*q2*q2 + 2*q2*ax + 4*q0*q1*q1 - 2*q1*ay;
  s1 = 4*q1*q3*q3 - 2*q3*ax + 4*q0*q0*q1 - 2*q0*ay;
  s2 = 4*q0*q0*q2 + 2*q0*ax + 4*q2*q3*q3 - 2*q3*ay;
  s3 = 4*q1*q1*q3 - 2*q1*ax + 4*q2*q2*q3 - 2*q2*ay;

  recipNorm = 1.0f / sqrt(s0*s0 + s1*s1 + s2*s2 + s3*s3);
  s0 *= recipNorm;
  s1 *= recipNorm;
  s2 *= recipNorm;
  s3 *= recipNorm;

  qDot1 -= beta * s0;
  qDot2 -= beta * s1;
  qDot3 -= beta * s2;
  qDot4 -= beta * s3;

  q0 += qDot1 * dt;
  q1 += qDot2 * dt;
  q2 += qDot3 * dt;
  q3 += qDot4 * dt;

  recipNorm = 1.0f / sqrt(q0*q0 + q1*q1 + q2*q2 + q3*q3);
  q0 *= recipNorm;
  q1 *= recipNorm;
  q2 *= recipNorm;
  q3 *= recipNorm;
}

/* ================= INIT ================= */
void initMic() {
  i2s_config_t cfg = {
    .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_RX),
    .sample_rate = SAMPLE_RATE,
    .bits_per_sample = I2S_BITS_PER_SAMPLE_32BIT,
    .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT,
    .communication_format = I2S_COMM_FORMAT_STAND_I2S,
    .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
    .dma_buf_count = 8,
    .dma_buf_len = BUFFER_LEN,
    .use_apll = false
  };

  i2s_pin_config_t pin = {
    .bck_io_num = I2S_SCK,
    .ws_io_num = I2S_WS,
    .data_out_num = I2S_PIN_NO_CHANGE,
    .data_in_num = I2S_SD
  };

  i2s_driver_install(I2S_PORT, &cfg, 0, NULL);
  i2s_set_pin(I2S_PORT, &pin);
  i2s_zero_dma_buffer(I2S_PORT);
}

/* ================= UPDATES ================= */
void updateIMU() {
  if (millis() - lastIMUTime < 20) return;
  float dt = (millis() - lastIMUTime) / 1000.0;
  lastIMUTime = millis();

  int16_t ax, ay, az, gx, gy, gz;
  mpu.getMotion6(&ax, &ay, &az, &gx, &gy, &gz);

  state.ax = ax / 16384.0;
  state.ay = ay / 16384.0;
  state.az = az / 16384.0;

  state.gx = gx / 131.0;
  state.gy = gy / 131.0;
  state.gz = gz / 131.0;

  madgwickUpdate(
    radians(state.gx),
    radians(state.gy),
    radians(state.gz),
    state.ax,
    state.ay,
    state.az,
    dt
  );

  state.roll  = atan2(2*(q0*q1 + q2*q3), 1 - 2*(q1*q1 + q2*q2)) * 57.2958;
  state.pitch = asin(2*(q0*q2 - q3*q1)) * 57.2958;
  state.yaw   = atan2(2*(q0*q3 + q1*q2), 1 - 2*(q2*q2 + q3*q3)) * 57.2958;
}

void updateAltitude() {
  static uint32_t t;
  if (millis() - t < 100) return;
  t = millis();

  uint16_t mm = vl53.read();
  if (mm > 0 && mm < 4000) state.altitude_cm = mm / 10;
}

void updateGPS() {
  while (Serial2.available()) gps.encode(Serial2.read());
  state.gps_fix = gps.location.isValid();
  if (state.gps_fix) {
    state.lat = gps.location.lat();
    state.lon = gps.location.lng();
    state.speed = gps.speed.kmph();
  }
}

void updateAudio() {
  static uint32_t t;
  if (millis() - t < 50) return;
  t = millis();

  size_t bytes_read;
  i2s_read(I2S_PORT, samples, sizeof(samples), &bytes_read, 10);

  int n = bytes_read / sizeof(int32_t);
  double sum = 0;
  for (int i = 0; i < n; i++) {
    double v = samples[i] >> 14;
    sum += v * v;
  }
  state.audio_rms = sqrt(sum / max(1, n));
  state.audio_spike = state.audio_rms > NOISE_THRESHOLD;
}

void updateHealth() {
  state.uptime_ms = millis();
  state.heap_free = ESP.getFreeHeap();
}

/* ================= HTTP ================= */
void handleData() {
  String j;
  j.reserve(720);

  j += "{";

  /* IMU RAW */
  j += "\"imu\":{";
  j += "\"ax\":"+String(state.ax,2)+",";
  j += "\"ay\":"+String(state.ay,2)+",";
  j += "\"az\":"+String(state.az,2)+",";
  j += "\"gx\":"+String(state.gx,2)+",";
  j += "\"gy\":"+String(state.gy,2)+",";
  j += "\"gz\":"+String(state.gz,2)+"},";

  /* ORIENTATION (EULER – KEEP FOR DEBUG) */
  j += "\"orientation\":{";
  j += "\"pitch\":"+String(state.pitch,1)+",";
  j += "\"roll\":"+String(state.roll,1)+",";
  j += "\"yaw\":"+String(state.yaw,1)+"},";

  /* 🔥 QUATERNION (THIS IS THE IMPORTANT PART) */
  j += "\"quat\":{";
  j += "\"w\":"+String(q0,6)+",";
  j += "\"x\":"+String(q1,6)+",";
  j += "\"y\":"+String(q2,6)+",";
  j += "\"z\":"+String(q3,6)+"},";

  /* ALTITUDE */
  j += "\"altitude_cm\":"+String(state.altitude_cm)+",";

  /* GPS */
  j += "\"gps\":{";
  j += "\"lat\":"+String(state.lat,6)+",";
  j += "\"lon\":"+String(state.lon,6)+",";
  j += "\"speed\":"+String(state.speed,1)+",";
  j += "\"fix\":" + String(state.gps_fix ? "true" : "false") + "},";

  /* AUDIO */
  j += "\"audio\":{";
  j += "\"rms\":"+String(state.audio_rms,2)+",";
  j += "\"spike\":" + String(state.audio_spike ? "true" : "false") + "},";

  /* HEALTH */
  j += "\"health\":{";
  j += "\"uptime_ms\":"+String(state.uptime_ms)+",";
  j += "\"heap_free\":"+String(state.heap_free)+"}";

  j += "}";

  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.send(200, "application/json", j);
}

/* ================= SETUP ================= */
void setup() {
  Serial.begin(115200);
  delay(1000);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) delay(300);

  Wire.begin(21, 22);
  Wire.setClock(100000);

  mpu.initialize();
  mpu.setSleepEnabled(false);

  vl53.init();
  vl53.setDistanceMode(VL53L1X::Long);
  vl53.startContinuous(100);

  Serial2.begin(9600, SERIAL_8N1, 16, 17);

  initMic();

  lastIMUTime = millis();

  server.on("/data", handleData);
  server.begin();
}

/* ================= LOOP ================= */
void loop() {
  server.handleClient();
  updateIMU();
  updateAltitude();
  updateGPS();
  updateAudio();
  updateHealth();
}
