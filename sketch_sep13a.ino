#include <esp_now.h>
#include <WiFi.h>

// Corrected MAC Address format with 0x prefixes and commas
uint8_t receiverAddress[] = {0x8C, 0x94, 0xDF, 0x45, 0x08, 0x44}; 

typedef struct struct_message {
  int water_level;
} struct_message;

struct_message myData;

void setup() {
  // Configure pins for the water probes
  pinMode(1, INPUT_PULLUP);
  pinMode(2, INPUT_PULLUP);
  pinMode(3, INPUT_PULLUP);
  pinMode(4, INPUT_PULLUP);

  // Initialize Wi-Fi in station mode for ESP-NOW
  WiFi.mode(WIFI_STA);
  
  // If ESP-NOW fails to start, sleep for 10 seconds and reboot to try again
  if (esp_now_init() != ESP_OK) {
    esp_sleep_enable_timer_wakeup(10e6); 
    esp_deep_sleep_start();
  }
  
  // Register the receiver
  esp_now_peer_info_t peerInfo;
  memcpy(peerInfo.peer_addr, receiverAddress, 6);
  peerInfo.channel = 0;  
  peerInfo.encrypt = false;
  esp_now_add_peer(&peerInfo);

  // Read water level (wires pull LOW when touching water)
  if(digitalRead(4) == LOW) {
    myData.water_level = 100;
  }
  else if(digitalRead(3) == LOW) {
    myData.water_level = 75;
  }
  else if(digitalRead(2) == LOW) {
    myData.water_level = 50;
  }
  else if(digitalRead(1) == LOW) {
    myData.water_level = 25;
  }
  else {
    myData.water_level = 0;
  }

  // Send data to receiver
  esp_now_send(receiverAddress, (uint8_t *) &myData, sizeof(myData));
  
  // Short delay to ensure the wireless packet finishes transmitting
  delay(100);
  
  // Enter Deep Sleep for 5 minutes (300,000,000 microseconds)
  esp_sleep_enable_timer_wakeup(300e6);
  esp_deep_sleep_start();
}

void loop() {
  // Empty, system wakes up and runs setup() again
}