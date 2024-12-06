#include <esp_now.h>
#include <WiFi.h>
#include <VescUart.h>

// PPM and LED pins
const int ppmPin = 22;  // PPM signal pin
const int led1 = 32;
const int led2 = 33;
const int led3 = 25;
const int led4 = 26;
const int ledPin1 = 5;
const int ledPin2 = 18;
const int ledPin3 = 19;

// VESC settings
VescUart UART;

// ESP-NOW variables
bool acc_button_status = false;  // Accelerator button state
bool newDataReceived = false;    // Flag for new ESP-NOW data
int currentMode = 1;             // Default mode

// Data structure for receiving ESP-NOW data
struct StatusData {
  bool acceleratorPressed;
  int mode;
};

// Callback function for receiving ESP-NOW data
void onDataRecv(const esp_now_recv_info *info, const uint8_t *data, int len) {
  StatusData receivedStatus;
  memcpy(&receivedStatus, data, sizeof(receivedStatus));

  acc_button_status = receivedStatus.acceleratorPressed;
  currentMode = receivedStatus.mode;
  newDataReceived = true;

  Serial.print("Received Mode: ");
  Serial.println(currentMode);
  Serial.print("Accelerator Pressed: ");
  Serial.println(acc_button_status);
}

// Function to send PPM signal to the motor controller
void sendPpmSignal(int ppmValue) {
  digitalWrite(ppmPin, HIGH);
  delayMicroseconds(ppmValue);
  digitalWrite(ppmPin, LOW);
}

// Function to control the motor based on mode and accelerator status
void handleMotorControl(int mode, bool acceleratorPressed) {
  int ppmValue = 600;  // Default idle value

  if (acceleratorPressed) {
    switch (mode) {
      case 1:
        ppmValue = 930;  // Mode 1 PPM value
        break;
      case 2:
        ppmValue = 1120;  // Mode 2 PPM value
        break;
      case 3:
        ppmValue = 1270;  // Mode 3 PPM value
        break;
    }
  }

  sendPpmSignal(ppmValue);  // Send the calculated PPM value
}

// Function to update LEDs based on the current mode
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

void setup() {
  Serial.begin(115200);
  pinMode(ppmPin, OUTPUT);
  pinMode(ledPin1, OUTPUT);
  pinMode(ledPin2, OUTPUT);
  pinMode(ledPin3, OUTPUT);
  pinMode(led1, OUTPUT);
  pinMode(led2, OUTPUT);
  pinMode(led3, OUTPUT);
  pinMode(led4, OUTPUT);
  UART.setSerialPort(&Serial);
  WiFi.mode(WIFI_STA);

  // Initialize ESP-NOW
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }
  esp_now_register_recv_cb(onDataRecv);
  updateModeLEDs();
  Serial.println("Setup complete.");
}

void loop() {
  // Update VESC values for RPM and voltage monitoring
  if (UART.getVescValues()) {
    Serial.print("RPM: ");
    Serial.println(UART.data.rpm);
    Serial.print("Battery Voltage: ");
    Serial.println(UART.data.inpVoltage);
    if (newDataReceived) {
      newDataReceived = false;
      updateModeLEDs();
      handleMotorControl(currentMode, acc_button_status);
    }
  } else {
    Serial.println("Failed to read VESC values");
  }
}
