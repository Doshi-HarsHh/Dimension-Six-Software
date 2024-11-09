//Board: LOLIN(WEMOS) DI R2 & mini
#include <ESP8266WiFi.h>
#include <espnow.h>

// Pin Definitions
#define INC_BUTTON D3  // Increment button
#define DEC_BUTTON D4  // Decrement button
#define ACC_BUTTON D1  // Accelerator button

// Variables
uint8_t mode = 1;  // Start in Mode 1
bool acc_button_state = false;
// uint8_t peerAddress[] = { 0xCC, 0xDB, 0xA7, 0x3F, 0xBC, 0x2C };  // Peer esp mac add
uint8_t peerAddress[] = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };  // Broadcast

typedef struct struct_message {
  uint8_t mode;
  bool acc_button_state;
} struct_message;

struct_message dataToSend;

// initialize ESP-NOW
void initESPNow() {
  WiFi.mode(WIFI_STA);
  if (esp_now_init() != 0) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }
  esp_now_set_self_role(ESP_NOW_ROLE_CONTROLLER);
  esp_now_add_peer(peerAddress, ESP_NOW_ROLE_SLAVE, 1, NULL, 0);
}

//send data to ESP32
void sendData() {
  dataToSend.mode = mode;
  dataToSend.acc_button_state = acc_button_state;
  esp_now_send(peerAddress, (uint8_t *)&dataToSend, sizeof(dataToSend));
}

// read button states and update mode
void updateMode() {
  static bool inc_last_state = HIGH;
  static bool dec_last_state = HIGH;
  bool inc_state = digitalRead(INC_BUTTON);
  bool dec_state = digitalRead(DEC_BUTTON);

  
  if (inc_state == LOW && inc_last_state == HIGH) {
    if (mode < 3) {
      mode++;
      Serial.print("Mode incremented to: ");
      Serial.println(mode);
      sendData();
    }
  }
  inc_last_state = inc_state;

  
  if (dec_state == LOW && dec_last_state == HIGH) {
    if (mode > 1) {
      mode--;
      Serial.print("Mode decremented to: ");
      Serial.println(mode);
      sendData();  // Send updated mode
    }
  }
  dec_last_state = dec_state;
}

// check accelerator button state
void checkAccButton() {
  bool acc_state = digitalRead(ACC_BUTTON);
  if (acc_state != acc_button_state) {
    acc_button_state = acc_state;
    Serial.print("Accelerator button state: ");
    Serial.println(acc_button_state ? "Released" : "Pressed");
    sendData();  // Send accelerator button state
  }
}

void setup() {
  Serial.begin(115200);
  pinMode(INC_BUTTON, INPUT_PULLUP);
  pinMode(DEC_BUTTON, INPUT_PULLUP);
  pinMode(ACC_BUTTON, INPUT_PULLUP);
  initESPNow();
  Serial.println("ESP-NOW initialized");
  Serial.println(WiFi.macAddress());
}

void loop() {
  updateMode();
  checkAccButton();
  delay(100); 
}
