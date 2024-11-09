#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>
#include <VescUart.h>

// BLE settings
BLEServer *pServer = nullptr;
BLECharacteristic *pCharacteristic = nullptr;
bool deviceConnected = false;
const char *BLE_DEVICE_NAME = "esp32";

// PPM settings
const int ppmPin = 22;

// LED settings
const int led1 = 32;
const int led2 = 33;
const int led3 = 25;
const int led4 = 26;

// VESC settings
VescUart UART;

// Global variables
int rpm = 0.00;
float inVOL = 42.0;
int currentMode = 1;
const int buttonPin = 21;

// LED mode indicators
const int ledPin1 = 5;
const int ledPin2 = 18;
const int ledPin3 = 19;

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

void setup() {
  Serial.begin(115200);

  // Initialize BLE
  BLEDevice::init(BLE_DEVICE_NAME);
  pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());

  BLEService *pService = pServer->createService(SERVICE_UUID);
  pCharacteristic = pService->createCharacteristic(
    CHARACTERISTIC_UUID,
    BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_WRITE);
  pCharacteristic->addDescriptor(new BLE2902());
  pService->start();

  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pAdvertising->start();

  pinMode(ppmPin, OUTPUT);
  UART.setSerialPort(&Serial);
  pinMode(buttonPin, INPUT_PULLUP);
  pinMode(ledPin1, OUTPUT);
  pinMode(ledPin2, OUTPUT);
  pinMode(ledPin3, OUTPUT);
  pinMode(led1, OUTPUT);
  pinMode(led2, OUTPUT);
  pinMode(led3, OUTPUT);
  pinMode(led4, OUTPUT);
  updateModeLEDs();

  Serial.println("Bluetooth device started, you can connect to it from your phone!");
}

void loop() {
  if (digitalRead(buttonPin) == LOW) {
    delay(300);  // Debouncing delay
    currentMode = (currentMode % 3) + 1;
    updateModeLEDs();
    Serial.print("Changed to Mode ");
    Serial.println(currentMode);
  }

  if (UART.getVescValues()) {
    rpm = UART.data.rpm;
    inVOL = UART.data.inpVoltage;
    handleBatteryStatus(inVOL);
    handleModeAndPpm(rpm, currentMode);
  } else {
    Serial.println("Failed to get data from VESC");
  }

  if (deviceConnected) {
    handleBluetoothData();
  }
}

void handleBatteryStatus(float batteryVoltage) {
  float batteryPercentage = (batteryVoltage - 30.0) / (42.0 - 30.0) * 100.0;
  batteryPercentage = constrain(batteryPercentage, 0, 100);

  if (batteryPercentage >= 75.0) {
    digitalWrite(led1, HIGH);
    digitalWrite(led2, HIGH);
    digitalWrite(led3, HIGH);
    digitalWrite(led4, HIGH);
  } else if (batteryPercentage >= 50.0) {
    digitalWrite(led1, HIGH);
    digitalWrite(led2, HIGH);
    digitalWrite(led3, HIGH);
    digitalWrite(led4, LOW);
  } else if (batteryPercentage >= 25.0) {
    digitalWrite(led1, HIGH);
    digitalWrite(led2, HIGH);
    digitalWrite(led3, LOW);
    digitalWrite(led4, LOW);
  } else if (batteryPercentage > 0.0) {
    digitalWrite(led1, HIGH);
    digitalWrite(led2, LOW);
    digitalWrite(led3, LOW);
    digitalWrite(led4, LOW);
  } else {
    digitalWrite(led1, LOW);
    digitalWrite(led2, LOW);
    digitalWrite(led3, LOW);
    digitalWrite(led4, LOW);
  }

  Serial.print("Battery Voltage: ");
  Serial.print(batteryVoltage);
  Serial.print(" Battery Percentage: ");
  Serial.print(batteryPercentage);
  Serial.println(" %");
}

void handleModeAndPpm(int rpm, int mode) {
  int ppmValue = 600;
  if (rpm > 2500) {
    switch (mode) {
      case 1:
        ppmValue = 930;
        break;
      case 2:
        ppmValue = 1120;
        break;
      case 3:
        ppmValue = 1270;
        break;
    }
  } else {
    ppmValue = 600;
  }
  sendPpmSignal(ppmValue);

  Serial.print("RPM: ");
  Serial.print(rpm);
  Serial.print(", PPM Value (Mode ");
  Serial.print(mode);
  Serial.print("): ");
  Serial.println(ppmValue);
}

void sendPpmSignal(int ppmValue) {
  digitalWrite(ppmPin, HIGH);
  delayMicroseconds(ppmValue);
  digitalWrite(ppmPin, LOW);
}

void updateModeLEDs() {
  digitalWrite(ledPin1, LOW);
  digitalWrite(ledPin2, LOW);
  digitalWrite(ledPin3, LOW);
  switch (currentMode) {
    case 1:
      digitalWrite(ledPin1, HIGH);
      break;
    case 2:
      digitalWrite(ledPin2, HIGH);
      break;
    case 3:
      digitalWrite(ledPin3, HIGH);
      break;
  }
}

void handleBluetoothData() {
  String data = "{\"rpm\":" + String(rpm) + ", \"battery\":" + String(inVOL, 2) + ", \"mode\":" + String(currentMode) + "}";
  pCharacteristic->setValue(data);
  pCharacteristic->notify();  // Send data to client
  Serial.print("Sent data: ");
  Serial.print(data);
}

void updateStatus(int index, bool status) {
  switch (index) {
    case 1:
      digitalWrite(led1, status);
      break;
    case 2:
      digitalWrite(led2, status);
      break;
    case 3:
      digitalWrite(led3, status);
      break;
    case 4:
      digitalWrite(led4, status);
      break;
    default:
      break;
  }
}
