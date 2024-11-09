#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>
#include <esp_now.h>
#include <WiFi.h>
#include <VescUart.h>

const int batt_led1 PROGMEM= 32;  
const int batt_led2 PROGMEM= 33;
const int batt_led3 PROGMEM= 25;
const int batt_led4 PROGMEM= 26;

// PPM pin
const int ppmPin PROGMEM= 22;  

// Mode indicator LED pins
const int Mode1_led PROGMEM= 5;
const int Mode2_led PROGMEM= 18;
const int Mode3_led PROGMEM= 19;

VescUart VESC;
int rpm = 0.00;
float inVOL = 38.0;
int currentMode;
bool acc_button_status = false;
float batteryCurrent;
bool buttonState;
// BLE
BLEServer *pServer = nullptr;
BLECharacteristic *pCharacteristic = nullptr;
bool deviceConnected = false;
const char *BLE_DEVICE_NAME = "Hook_2";
// BLE UUIDs
#define SERVICE_UUID "12345678-1234-1234-1234-123456789012"
#define CHARACTERISTIC_UUID "87654321-4321-4321-4321-210987654321"

class MyServerCallbacks : public BLEServerCallbacks {
  void onConnect(BLEServer *pServer) {
    deviceConnected = true;
  }
  void onDisconnect(BLEServer *pServer) {
    deviceConnected = false;
  }
};

// Structure for incoming ESP-NOW data
typedef struct struct_message {
  uint8_t mode;
  bool acc_button_state;
} struct_message;

struct_message incomingData;

// ESP-NOW Wemos D1 Mini MAC address
uint8_t senderMacAddress[] = { 0x68, 0xC6, 0x3A, 0xF3, 0xE7, 0xF2 };

void onDataReceive(const esp_now_recv_info *info, const uint8_t *data, int len) {
  memcpy(&incomingData, data, sizeof(incomingData));

  currentMode = incomingData.mode;
  acc_button_status = incomingData.acc_button_state;


  Serial.print("Received Mode: ");
  Serial.println(currentMode);
  Serial.print("Accelerator Button Status: ");
  Serial.println(acc_button_status ? "Pressed" : "Released");

  updateModeLEDs();
}

void updateModeLEDs() {
  digitalWrite(Mode1_led, LOW);
  digitalWrite(Mode2_led, LOW);
  digitalWrite(Mode3_led, LOW);
  switch (currentMode) {
    case 1:
      digitalWrite(Mode1_led, HIGH);
      break;
    case 2:
      digitalWrite(Mode2_led, HIGH);
      break;
    case 3:
      digitalWrite(Mode3_led, HIGH);
      break;
  }
}

void handleBluetoothData() {
  String data = "{\"rpm\":" + String(rpm) + ", \"battery\":" + String(inVOL, 2) + ", \"mode\":" + String(currentMode) + "}";
  pCharacteristic->setValue(data);
  pCharacteristic->notify();
  Serial.print("Sent data: ");
  Serial.print(data);
}

void sendPpm(int mode) {
  int ppmValue = 600;
  // VESC.setCurrent(0);
  // Set PPM values based on mode
  switch (mode) {
    case 1:
      ppmValue = 960;
      VESC.setCurrent(12);
      break;
    case 2:
      ppmValue = 1120;
      VESC.setCurrent(15);
      break;
    case 3:
      ppmValue = 1300;
      VESC.setCurrent(18);
      break;
  }
  // Send PPM pulse  
  digitalWrite(ppmPin, HIGH);
  delayMicroseconds(ppmValue);
  digitalWrite(ppmPin, LOW);
}

void handleBatteryStatus(float batteryVoltage) {
  float batteryPercentage = (batteryVoltage - 30.0) / (42.0 - 30.0) * 100.0;
  batteryPercentage = constrain(batteryPercentage, 0, 100);

  digitalWrite(batt_led1, batteryPercentage >= 25.0);
  digitalWrite(batt_led2, batteryPercentage >= 50.0);
  digitalWrite(batt_led3, batteryPercentage >= 75.0);
  digitalWrite(batt_led4, batteryPercentage >= 100.0);

  Serial.print("Battery Voltage: ");
  Serial.print(batteryVoltage);
  Serial.print(" Battery Percentage: ");
  Serial.print(batteryPercentage);
  Serial.println(" %");
}
