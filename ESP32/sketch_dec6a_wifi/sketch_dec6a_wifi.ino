  // put your main code here, to run repeatedly:
#include <WiFi.h>
#include <WebServer.h>
int count=0;
const char* ssid     = "ESP32_AP";
const char* password = "12345678";

WebServer server(80);

void handleRoot() {
  server.send(200, "text/plain", "Hello from ESP32 AP");
}

void setup() {
  Serial.begin(115200);
  delay(1000);

  WiFi.mode(WIFI_MODE_AP);
  WiFi.softAP(ssid, password);

  IPAddress IP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(IP);

  server.on("/", handleRoot);
  server.begin();
  Serial.println("HTTP server started");
}

void loop() {
  if(count<=3){
    server.handleClient();
    count++;
  }
   Serial.println(count);
  }


