#include <WiFi.h>
#include <esp_now.h>
#include <ESP32Servo.h>

Servo s1,s2,s3,s4,s360;

#define SERVO1_PIN 32
#define SERVO2_PIN 33
#define SERVO3_PIN 25
#define SERVO4_PIN 26
#define SERVO360_PIN 14
#define FORCE_PIN 35
uint8_t espA_mac[] = {0xD4,0xE9,0xF4,0xBC,0x5A,0xF8};

typedef struct {
   int16_t mpu1_ax, mpu1_ay,mpu1_az,mpu1_gx,mpu1_gy,mpu1_gz;
  int16_t mpu2_ax, mpu2_ay,mpu2_az,mpu2_gx,mpu2_gy,mpu2_gz;
  int flex;
} MotionData;

typedef struct {
  int force;
} ForceData;

MotionData rxData;
ForceData txData;

void onReceive(const esp_now_recv_info *info, const uint8_t *data, int len) {
  memcpy(&rxData, data, sizeof(rxData));
}

int mapServo(int v,int inMin,int inMax,int outMin,int outMax){
  v = constrain(v,inMin,inMax);
  return map(v,inMin,inMax,outMin,outMax);
}


void setup() {
  Serial.begin(115200);

  s1.attach(SERVO1_PIN);
  s2.attach(SERVO2_PIN);
  s3.attach(SERVO3_PIN);
  s4.attach(SERVO4_PIN);
  s360.attach(SERVO360_PIN);
 WiFi.mode(WIFI_STA);
  esp_now_init();

  esp_now_register_recv_cb(onReceive);

  esp_now_peer_info_t peer = {};
  memcpy(peer.peer_addr, espA_mac, 6);
  peer.channel = 0;
  peer.encrypt = false;
  esp_now_add_peer(&peer);
}
  

 void loop(){
 // reverse speed
  // 360 SERVO CONTROL
if (rxData.mpu2_ay >= 4000) {
  s360.write(60);   // Forward rotation
}
else if (rxData.mpu2_ax <= 1000) {
  s360.write(120);  // Backward rotation
}
else {
  s360.write(90);   // Stop
}
//S3 SERVO
if (rxData.mpu1_az <= -2000 && rxData.mpu1_az >= -16000) {
  s3.write(map(rxData.mpu1_az, -2000, -16000, 60, 140));
}
else if (rxData.mpu1_az >= -100 && rxData.mpu1_az <= 16000) {
  s3.write(map(rxData.mpu1_az, 16000, 6000, 140, 60));
}
else {
  
}
//S2 SERVO
if (rxData.mpu2_az >= 4000 && rxData.mpu2_az <= 16000) {
  s2.write(map(rxData.mpu2_az, 4000, 16000, 120, 180));
}
else if (rxData.mpu2_az <= 3500 && rxData.mpu2_az >= -16000) {
  s2.write(map(rxData.mpu2_az, 3500, -16000, 180, 120));
}
else {
  
}
//S1 SERVO
if (rxData.mpu2_gy <= 4000 && rxData.mpu2_gy >=-5000) {
  s1.write(map(rxData.mpu2_gy, 4000, -5000, 0, 30));
}
else if (rxData.mpu2_gy >= 3500 && rxData.mpu2_gy <= 7500) {
  s1.write(map(rxData.mpu2_gy, 3500, 7500, 30, 0));
}
else {
  
}
// S4 SERVO (Flex Control)

if (rxData.flex >= 0 && rxData.flex <= 40) {
  s4.write(map(rxData.flex, 0, 40, 175, 120));
}
  // Stop
  
  txData.force = analogRead(FORCE_PIN);
  esp_now_send(espA_mac,(uint8_t*)&txData,sizeof(txData));

  Serial.print("Force TX: "); Serial.println(txData.force);
  delay(20);
  
}