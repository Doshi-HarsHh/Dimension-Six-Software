#include "includes.h"

void setup() {
  pinMode(ppmPin, OUTPUT);
  pinMode(Mode1_led, OUTPUT);
  pinMode(Mode2_led, OUTPUT);
  pinMode(Mode3_led, OUTPUT);
  pinMode(batt_led1, OUTPUT);
  pinMode(batt_led2, OUTPUT);
  pinMode(batt_led3, OUTPUT);
  pinMode(batt_led4, OUTPUT);

  Serial.begin(115200);
  VESC.setSerialPort(&Serial);
  // Initialize BLE
  BLEDevice::init(BLE_DEVICE_NAME);
  pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());
  BLEService *pService = pServer->createService(SERVICE_UUID);
  pCharacteristic = pService->createCharacteristic(CHARACTERISTIC_UUID, BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_WRITE);
  pCharacteristic->addDescriptor(new BLE2902());
  pService->start();
  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pAdvertising->start();
  // Initialize ESP-NOW
  WiFi.mode(WIFI_STA);
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }
  // Add peer information
  esp_now_peer_info_t peerInfo;
  memcpy(peerInfo.peer_addr, senderMacAddress, 6);
  peerInfo.channel = 0;
  peerInfo.encrypt = false;
  if (esp_now_add_peer(&peerInfo) != ESP_OK) {
    Serial.println("Failed to add peer");
    return;
  }
  // Register receive callback function
  esp_now_register_recv_cb(onDataReceive);
  updateModeLEDs();
  Serial.println("Bluetooth device started, you can connect to it from your phone!");
}

void loop() {
  if (VESC.getVescValues()) {
    rpm = VESC.data.rpm;
    inVOL = VESC.data.inpVoltage;
    batteryCurrent = VESC.data.avgInputCurrent;
    Serial.print("Battery Current (avgInputCurrent): ");
    Serial.print(batteryCurrent);
    Serial.println("A");
  } else {
    Serial.println("Failed to get data from VESC");
  }

  if (deviceConnected) {
    handleBluetoothData();
  }

  
  while (!acc_button_status) {

    sendPpm(currentMode); 
    VESC.getVescValues();  
    Serial.print("Running motor at mode: ");
    Serial.println(currentMode);
    Serial.println("Accelerator button pressed, sending PPM signal.");

    if (acc_button_status) {
      
      VESC.setCurrent(0);
      Serial.println("Accelerator button released, motor stopped.");
      break;
    }
  }
}
