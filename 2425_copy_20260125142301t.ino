#include <WiFi.h>
#include <esp_now.h>
#include <Wire.h>
#include <MPU6050.h>

MPU6050 mpu1(0x68);
MPU6050 mpu2(0x69);

#define FLEX_PIN 35
#define COIN1_PIN 13
#define COIN2_PIN 14

//  PUT ESP B MAC HERE
uint8_t espB_mac[] = {0xA4,0xF0,0x0F,0x66,0x79,0x3C};

typedef struct {
  int16_t mpu1_ax, mpu1_ay,mpu1_az,mpu1_gx,mpu1_gy,mpu1_gz;
  int16_t mpu2_ax, mpu2_ay,mpu2_az,mpu2_gx,mpu2_gy,mpu2_gz;
  int flex;
} MotionData;

typedef struct {
  int force;
} ForceData;

MotionData txData;
ForceData rxData;

void onReceive(const esp_now_recv_info *info, const uint8_t *data, int len) {
  memcpy(&rxData, data, sizeof(rxData));

  if (rxData.force > 2000) {
    digitalWrite(COIN1_PIN, HIGH);
    digitalWrite(COIN2_PIN, HIGH);
  } else {
    digitalWrite(COIN1_PIN, LOW);
    digitalWrite(COIN2_PIN, LOW);
  }
}

void setup() {
  Serial.begin(115200);

  pinMode(COIN1_PIN, OUTPUT);
  pinMode(COIN2_PIN, OUTPUT);

  Wire.begin(21,22);
  mpu1.initialize();
  mpu2.initialize();

  WiFi.mode(WIFI_STA);
  esp_now_init();

  esp_now_register_recv_cb(onReceive);

  esp_now_peer_info_t peer = {};
  memcpy(peer.peer_addr, espB_mac, 6);
  peer.channel = 0;
  peer.encrypt = false;
  esp_now_add_peer(&peer);
}

void loop() {
  int16_t az,ay,ax,gx,gy,gz;

  mpu1.getMotion6(
  &txData.mpu1_ax,
  &txData.mpu1_ay,
  &txData.mpu1_az,
  &txData.mpu1_gx,
  &txData.mpu1_gy,
  &txData.mpu1_gz
  );
  mpu2.getMotion6(
  &txData.mpu2_ax,
  &txData.mpu2_ay,
  &txData.mpu2_az,
  &txData.mpu2_gx,
  &txData.mpu2_gy,
  &txData.mpu2_gz
  );
  txData.flex = analogRead(FLEX_PIN);

  esp_now_send(espB_mac, (uint8_t*)&txData, sizeof(txData));

  Serial.print("MPU1 AX: "); Serial.print(txData.mpu1_ax);
  Serial.print(" MPU2 AX: "); Serial.print(txData.mpu2_ax);
  Serial.print("MPU1 AY: "); Serial.print(txData.mpu1_ay);
  Serial.print(" MPU2 AY: "); Serial.print(txData.mpu2_ay);
  Serial.print("MPU1 AZ: "); Serial.print(txData.mpu1_az);
  Serial.print(" MPU2 AZ: "); Serial.print(txData.mpu2_az);
  Serial.print("MPU1 GX: "); Serial.print(txData.mpu1_az);
  Serial.print(" MPU2 GX: "); Serial.print(txData.mpu2_az);
  Serial.print("MPU1 GY: "); Serial.print(txData.mpu1_az);
  Serial.print(" MPU2 GY: "); Serial.print(txData.mpu2_az);
  Serial.print("MPU1 GZ: "); Serial.print(txData.mpu1_az);
  Serial.print(" MPU2 GZ: "); Serial.print(txData.mpu2_az);
  Serial.print(" FLEX: "); Serial.print(txData.flex);
  Serial.print(" FORCE RX: "); Serial.println(rxData.force);

  delay(200);
}