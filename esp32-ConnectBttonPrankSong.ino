#include <WiFi.h>
#include <DNSServer.h>
#include <WebServer.h>
#include <SD.h>
#include <SPI.h>

// Nastavenia pre SD kartu a WiFi
#define SD_CS_PIN 5                    // Pin pre SD kartu
#define SSID_NAME "Free WiFi"          // Názov WiFi siete
IPAddress APIP(172, 0, 0, 1);          // IP adresa ESP32

DNSServer dnsServer;
WebServer webServer(80);

// Funkcia na obsluhu MP3 súboru
void handleSong() {
    File file = SD.open("/Song.mp3");
    if (file) {
        webServer.streamFile(file, "audio/mpeg");
        file.close();
    } else {
        webServer.send(404, "text/plain", "MP3 súbor nebol nájdený.");
    }
}

// HTML obsah pre tlačidlo Connect
void handleConnectPage() {
    String html = "<!DOCTYPE html><html><head><title>Captive Portal</title>";
    html += "<meta name='viewport' content='width=device-width, initial-scale=1'>";
    html += "<style>body { display: flex; align-items: center; justify-content: center; height: 100vh; font-family: Arial; }";
    html += "button { font-size: 20px; padding: 10px; color: white; background-color: green; border: none; cursor: pointer; }</style></head>";
    html += "<body><button onclick='playSong()'>Connect</button>";
    html += "<script>function playSong() { var audio = new Audio(\"/song\"); audio.volume = 0.75; audio.play(); }</script>";
    html += "</body></html>";
    webServer.send(200, "text/html", html);
}

void setup() {
    // Inicializácia sériového portu a SD karty
    Serial.begin(115200);
    
    if (!SD.begin(SD_CS_PIN)) {
        Serial.println("Chyba pri otváraní SD karty!");
        return;
    }
    Serial.println("SD karta inicializovaná.");

    // Nastavenie WiFi v režime Access Point
    WiFi.mode(WIFI_AP);
    WiFi.softAPConfig(APIP, APIP, IPAddress(255, 255, 255, 0));
    WiFi.softAP(SSID_NAME);

    dnsServer.start(53, "*", APIP);  // DNS server smeruje všetky domény na IP adresu ESP32

    // Nastavenie HTTP endpointov
    webServer.on("/", handleConnectPage);  // Zobrazí stránku s tlačidlom Connect
    webServer.on("/song", handleSong);     // Endpoint pre MP3 súbor

    webServer.begin();
    Serial.println("HTTP server spustený...");
}

void loop() {
    dnsServer.processNextRequest();
    webServer.handleClient();
}
