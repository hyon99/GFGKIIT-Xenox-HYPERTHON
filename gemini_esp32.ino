#include <Arduino.h>
#include <Arduino.h> 
#include <WiFi.h> 
#include <HTTPClient.h> 
#include <ArduinoJson.h> 
#include <Wire.h> 
#include <U8g2lib.h> 
 
// Gemini API details 
const char* Gemini_Token = "AIzaSyCfrDnihja_D7ort4osuSErG8DMBJJBD1I"; 
const char* Gemini_Max_Tokens = "100"; 
 
String res = ""; 
 
// LED pin definitions 
const int greenLED = 4;     
const int redLED = 14;      
 
// OLED display setup 
#define SCREEN_WIDTH 128  
#define SCREEN_HEIGHT 64  
#define OLED_SDA 21 
#define OLED_SCL 23 
 
U8G2_SSD1306_128X64_NONAME_F_HW_I2C display(U8G2_R0, /* reset=*/ 
U8X8_PIN_NONE, OLED_SCL, OLED_SDA); 
 
String ssid = "";     

 
String password = "";   
 
void setup() { 
  Serial.begin(115200); 
 
  Wire.begin(OLED_SDA, OLED_SCL); 
 
  display.begin(); 
  display.clearBuffer(); 
  display.setFont(u8g2_font_5x8_tr); 
  display.drawStr(0, 10, "Initializing..."); 
  display.sendBuffer(); 
 
  pinMode(greenLED, OUTPUT); 
  pinMode(redLED, OUTPUT); 
 
  digitalWrite(greenLED, LOW); 
  digitalWrite(redLED, HIGH); 
 
  display.clearBuffer(); 
  display.drawStr(0, 10, "Enter WiFi SSID:"); 
  display.sendBuffer(); 
 
  Serial.println("Enter WiFi SSID:"); 
  while (!Serial.available()) {} 
  ssid = Serial.readStringUntil('\n'); 
  ssid.trim(); 
 
  display.clearBuffer(); 

 
  display.drawStr(0, 10, "Enter WiFi Password:"); 
  display.sendBuffer(); 
 
  Serial.println("Enter WiFi Password:"); 
  while (!Serial.available()) {} 
  password = Serial.readStringUntil('\n'); 
  password.trim(); 
 
  WiFi.mode(WIFI_STA); 
  WiFi.disconnect(); 
 
  WiFi.begin(ssid.c_str(), password.c_str()); 
  display.clearBuffer(); 
  display.drawStr(0, 10, "Connecting to WiFi..."); 
  display.sendBuffer(); 
 
  while (WiFi.status() != WL_CONNECTED) { 
    delay(1000); 
  } 
 
  Serial.println("Connected"); 
  Serial.print("IP address: "); 
  Serial.println(WiFi.localIP()); 
 
  digitalWrite(redLED, LOW); 
  digitalWrite(greenLED, HIGH); 
} 
 
void displayAnswer(String answer) { 

 
  display.clearBuffer(); 
  display.setFont(u8g2_font_5x8_tr); 
 
  int y = 10; 
  for (size_t i = 0; i < answer.length(); i += 21) { 
    display.drawStr(0, y, answer.substring(i, i + 21).c_str()); 
    y += 8; 
    if (y >= SCREEN_HEIGHT) { 
      break; 
    } 
  } 
  display.sendBuffer(); 
} 
 
void loop() { 
  digitalWrite(greenLED, HIGH); 
  digitalWrite(redLED, LOW); 
 
  while (!Serial.available()) {} 
 
  while (Serial.available()) { 
    char add = Serial.read(); 
    res = res + add; 
    delay(1); 
  } 
  int len = res.length(); 
  res = res.substring(0, (len - 1)); 
  res = "\"" + res + "\""; 
  
 
  Serial.println(""); 
  Serial.print("Asking Your Question: "); 
  Serial.println(res); 
 
  HTTPClient https; 
 
  if 
(https.begin("https://generativelanguage.googleapis.com/v1beta/model
 s/gemini-1.5-flash:generateContent?key=" + (String)Gemini_Token)) { 
    https.addHeader("Content-Type", "application/json"); 
    String payload = String("{\"contents\": [{\"parts\":[{\"text\":" 
+ res + "}]}],\"generationConfig\": {\"maxOutputTokens\": " + 
(String)Gemini_Max_Tokens + "}}"); 
 
    int httpCode = https.POST(payload); 
 
    if (httpCode == HTTP_CODE_OK || httpCode == 
HTTP_CODE_MOVED_PERMANENTLY) { 
      String payload = https.getString(); 
 
      DynamicJsonDocument doc(1024); 
      deserializeJson(doc, payload); 
 
      String Answer = 
doc["candidates"][0]["content"]["parts"][0]["text"]; 
      Answer.trim(); 
      String filteredAnswer = ""; 
      for (size_t i = 0; i < Answer.length(); i++) { 
        char c = Answer[i]; 
        if (isalnum(c) || isspace(c)) { 
          filteredAnswer += c; 
 
        } else { 
          filteredAnswer += ' '; 
        } 
      } 
      Answer = filteredAnswer; 
 
      digitalWrite(redLED, HIGH); 
      digitalWrite(greenLED, LOW); 
 
      displayAnswer(Answer); 
 
      delay(1000); 
    } else { 
      Serial.printf("[HTTPS] GET... failed, error: %s\n", 
https.errorToString(httpCode).c_str()); 
    } 
    https.end(); 
  } else { 
    Serial.printf("[HTTPS] Unable to connect\n"); 
  } 
 
  res = ""; 
  delay(1000); 
}
