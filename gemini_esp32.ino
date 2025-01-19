#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

// Gemini API details
const char* Gemini_Token = "AIzaSyCfrDnihja_D7ort4osuSErG8DMBJJBD1I";
const char* Gemini_Max_Tokens = "100";

String res = "";

// LED pin definitions
const int greenLED = 14;
const int redLED = 12;

String ssid = "";    // WiFi SSID
String password = "";  // WiFi password

void setup() {
  Serial.begin(115200);

  // Initialize LED pins
  pinMode(greenLED, OUTPUT);
  pinMode(redLED, OUTPUT);

  // Indicate not ready (red LED on)
  digitalWrite(greenLED, LOW);
  digitalWrite(redLED, HIGH);

  // Prompt user for WiFi credentials
  Serial.println("Enter WiFi SSID:");
  while (!Serial.available()) {
    // Wait for user input
  }
  ssid = Serial.readStringUntil('\n');
  ssid.trim(); // Remove any extra spaces or newlines

  Serial.println("Enter WiFi Password:");
  while (!Serial.available()) {
    // Wait for user input
  }
  password = Serial.readStringUntil('\n');
  password.trim(); // Remove any extra spaces or newlines

  WiFi.mode(WIFI_STA);
  WiFi.disconnect();

  while (!Serial)
    ;

  // Connect to WiFi
  WiFi.begin(ssid.c_str(), password.c_str());
  Serial.print("Connecting to ");
  Serial.println(ssid);

  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }

  Serial.println("connected");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  // Indicate ready (green LED on)
  digitalWrite(redLED, LOW);
  digitalWrite(greenLED, HIGH);
}

void loop() {
  Serial.println("");
  Serial.println("Ask your Question: ");
  
  // Indicate not ready (red LED on)
  digitalWrite(greenLED, HIGH);
  digitalWrite(redLED, LOW);

  while (!Serial.available())
    ;

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

  // Indicate communication in progress (red LED on)
  if (https.begin("https://generativelanguage.googleapis.com/v1beta/models/gemini-1.5-flash:generateContent?key=" + (String)Gemini_Token)) {  // HTTPS
    https.addHeader("Content-Type", "application/json");
    String payload = String("{\"contents\": [{\"parts\":[{\"text\":" + res + "}]}],\"generationConfig\": {\"maxOutputTokens\": " + (String)Gemini_Max_Tokens + "}}");

    int httpCode = https.POST(payload);

    if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY) {
      String payload = https.getString();

      DynamicJsonDocument doc(1024);
      deserializeJson(doc, payload);

      String Answer = doc["candidates"][0]["content"]["parts"][0]["text"];
      
      // Filter special characters, white spaces, and new line characters
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

      // Indicate ready (green LED on)
      digitalWrite(redLED, HIGH);
      digitalWrite(greenLED, LOW);

      Serial.println("");
      Serial.println("Here is your Answer: ");
      Serial.println("");
      Serial.println(Answer);
    } else {
      Serial.printf("[HTTPS] GET... failed, error: %s\n", https.errorToString(httpCode).c_str());
    }
    https.end();
  } else {
    Serial.printf("[HTTPS] Unable to connect\n");
  }

  res = "";
  delay(1000);
}
