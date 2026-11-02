#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>
#include <esp_now.h>
#include <LiquidCrystal_I2C.h>
#include <time.h>

const char* ssid = "XXXXXXXXXXXXXXXXXXXXXX";
const char* password = "XXXXXXXXXXXXXXX";
const char* botToken = "XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX";
const char* chatId = "XXXXXXXXXXXXXXXXXXXX";

// NTP Server settings for India Standard Time (IST = UTC + 5:30)
const char* ntpServer = "pool.ntp.org";
const long  gmtOffset_sec = 19800; 
const int   daylightOffset_sec = 0;

WiFiClientSecure client;
UniversalTelegramBot bot(botToken, client);
LiquidCrystal_I2C lcd(0x27, 16, 2); 

const int relayPin = 5;
const int btnOn = 12;
const int btnOff = 14;
int currentWaterLevel = 0;

typedef struct struct_message {
  int water_level;
} struct_message;
struct_message incomingData;

// Get current date and time in 12-Hour AM/PM format
String getCurrentTime() {
  struct tm timeinfo;
  if(!getLocalTime(&timeinfo)){
    return "Time not synchronized";
  }
  char timeStringBuff[50];
  // %I is 12-hour format, %p is AM/PM
  strftime(timeStringBuff, sizeof(timeStringBuff), "%d-%m-%Y %I:%M:%S %p", &timeinfo);
  return String(timeStringBuff);
}

void updateLCD() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Level: " + String(currentWaterLevel) + "%");
  lcd.setCursor(0, 1);
  lcd.print(digitalRead(relayPin) ? "Motor: ON" : "Motor: OFF");
}

void OnDataRecv(const esp_now_recv_info *info, const uint8_t *incomingDataBytes, int len) {
  memcpy(&incomingData, incomingDataBytes, sizeof(incomingData));
  currentWaterLevel = incomingData.water_level;
  
  // AUTOMATION: Auto-stop when 100% Full
  if(currentWaterLevel == 100 && digitalRead(relayPin) == HIGH) {
    digitalWrite(relayPin, LOW);
    String alert = "⚠️ Tank 100% FULL.\nAuto-stopped at: " + getCurrentTime();
    bot.sendMessage(chatId, alert, "");
  }
  
  // AUTOMATION: Auto-start when level is 25% or lower
  if(currentWaterLevel <= 25 && digitalRead(relayPin) == LOW) {
    digitalWrite(relayPin, HIGH);
    String alert = "⚠️ Tank level LOW (" + String(currentWaterLevel) + "%).\nAuto-started at: " + getCurrentTime();
    bot.sendMessage(chatId, alert, "");
  }
  
  updateLCD();
}

void setup() {
  pinMode(relayPin, OUTPUT);
  pinMode(btnOn, INPUT_PULLUP);
  pinMode(btnOff, INPUT_PULLUP);
  digitalWrite(relayPin, LOW);

  lcd.begin();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("Connecting Wi-Fi");

  WiFi.mode(WIFI_AP_STA);
  WiFi.begin(ssid, password);
  client.setCACert(TELEGRAM_CERTIFICATE_ROOT);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
  }

  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);

  esp_now_init();
  esp_now_register_recv_cb(OnDataRecv);
  updateLCD();
}

void loop() {
  // 1. Handle Manual Push Buttons (Hardware Override)
  if(digitalRead(btnOn) == LOW) {
    if (currentWaterLevel == 100) {
      bot.sendMessage(chatId, "⚠️ Cannot start. Tank is already 100% FULL.", "");
    } else {
      digitalWrite(relayPin, HIGH);
      bot.sendMessage(chatId, "🔧 Motor started MANUALLY via switch at:\n" + getCurrentTime(), "");
    }
    updateLCD();
    delay(1000); // Debounce delay
  }
  
  if(digitalRead(btnOff) == LOW) {
    digitalWrite(relayPin, LOW);
    updateLCD();
    bot.sendMessage(chatId, "🔧 Motor stopped MANUALLY via switch at:\n" + getCurrentTime(), "");
    delay(1000); // Debounce delay
  }

  // 2. Handle Telegram API Polling
  int numNewMessages = bot.getUpdates(bot.last_message_received + 1);
  for (int i = 0; i < numNewMessages; i++) {
    String text = bot.messages[i].text;
    String chat_id = String(bot.messages[i].chat_id);
    String keyboardJson = "[[\"Start Motor / மோட்டார் ஆன்\", \"Stop Motor / மோட்டார் ஆஃப்\"], [\"Check Status / நிலைமை\"]]";

    if (text == "/start") {
      bot.sendMessageWithReplyKeyboard(chat_id, "Water Monitor Active. Select command:", "", keyboardJson, true);
    }
    else if (text == "Start Motor / மோட்டார் ஆன்") {
      if (currentWaterLevel == 100) {
        bot.sendMessage(chat_id, "⚠️ Cannot start. Tank is already 100% FULL.", "");
      } else {
        digitalWrite(relayPin, HIGH);
        bot.sendMessage(chat_id, "Motor is ON ✅\n" + getCurrentTime(), "");
      }
      updateLCD();
    }
    else if (text == "Stop Motor / மோட்டார் ஆஃப்") {
      digitalWrite(relayPin, LOW);
      bot.sendMessage(chat_id, "Motor is OFF ❌\n" + getCurrentTime(), "");
      updateLCD();
    }
    else if (text == "Check Status / நிலைமை") {
      String statusMsg = "📅 Time: " + getCurrentTime() + 
                         "\n💧 Water Level: " + String(currentWaterLevel) + "%" +
                         "\n⚙️ Motor is " + (digitalRead(relayPin) ? "ON" : "OFF");
      bot.sendMessage(chat_id, statusMsg, "");
    }
  }
}
