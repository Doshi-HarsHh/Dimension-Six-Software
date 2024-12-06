#include <esp_now.h>
#include <WiFi.h>

uint8_t receiverMAC[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};  // Update with actual receiver MAC address

const int acceleratorPin = 5; // GPIO 5
const int incrementPin = 4;   // GPIO 4
const int decrementPin = 19;  // GPIO 19

int mode = 1; //inital mode

unsigned long lastModeChangeTime = 0;
const unsigned long debounceDelay = 300;  // Debounce delay to prevent multiple presses


struct StatusData {
  bool acceleratorPressed;  // Acc button status
  int mode;                 // Current mode
};

//instance of structure
StatusData status = {false, 1}; // Initial state: accelerator not pressed, mode is 1

void sendMessage(StatusData data) {
  esp_now_send(receiverMAC, (uint8_t *)&data, sizeof(data));  // Send the data struct
}

void setup() {
  pinMode(acceleratorPin, INPUT_PULLUP);
  pinMode(incrementPin, INPUT_PULLUP);
  pinMode(decrementPin, INPUT_PULLUP);
  digitalWrite(incrementPin,HIGH);
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);

  if (esp_now_init() != ESP_OK) {
    Serial.println("ESP-NOW initialization failed");
    return;
  }

  // Register the receiver peer
  esp_now_peer_info_t peerInfo;
  memcpy(peerInfo.peer_addr, receiverMAC, 6);
  peerInfo.channel = 0;
  peerInfo.encrypt = false;

  if (esp_now_add_peer(&peerInfo) != ESP_OK) {
    Serial.println("Failed to add peer");
    return;
  }


}

void loop() {
  Serial.print("INC BUTTON: ");
  Serial.println(digitalRead(incrementPin));
    Serial.print("DEC BUTTON: ");
  Serial.println(digitalRead(decrementPin));

  // Check if the accelerator button is pressed (low)
  status.acceleratorPressed = (digitalRead(acceleratorPin) == LOW);
  
  // Send the current accelerator and mode status
  sendMessage(status);
  delay(50);  // Sending rate
  if (digitalRead(decrementPin) == LOW && millis() - lastModeChangeTime > debounceDelay) {
    lastModeChangeTime = millis();
    if (mode > 1) {  // Only decrement if not at min mode
      mode--;
      status.mode = mode;
      sendMessage(status);  // Send updated mode
    }
  }
  // Handle mode increment
  if (digitalRead(incrementPin) == LOW && millis() - lastModeChangeTime > debounceDelay) {
    lastModeChangeTime = millis();
    if (mode < 3) {  // Only increment if not at max mode
      mode++;
      status.mode = mode;
      sendMessage(status);  // Send updated mode
    }
  }

  

}
