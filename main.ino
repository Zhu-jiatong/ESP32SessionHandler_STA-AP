#include <DNSServer.h>

#define DEBUG_SESSIONMANAGER

#include "src/ESP32SessionManager/ESPSessionManager.hpp"

AsyncWebServer server(80);
DNSServer dns_ap;
cst::ESPSessionManager session_manager;

void handleRoot(AsyncWebServerRequest *request);
void handleAuth(AsyncWebServerRequest *request);
void handleLogoff(AsyncWebServerRequest *request);

void setup()
{
    Serial.begin(115200);
    WiFi.disconnect(true, true);
    WiFi.softAP("ESP AP");
    WiFi.begin("brenda iPad Mini 3 (3)", "2fkmby2spezi8");
    while (WiFi.status() != WL_CONNECTED)
    {
        Serial.println("Connecting");
        delay(500);
    }
    Serial.printf("Connected to %s", WiFi.SSID());
    Serial.println(WiFi.localIP());

    server.on("/", handleRoot);
    server.on("/auth", handleAuth);
    server.on("/logoff", handleLogoff);
    server.begin();

    dns_ap.setTTL(300);
    Serial.println(dns_ap.start(53, "test.com", WiFi.softAPIP()));
}

void loop()
{
    dns_ap.processNextRequest();
}

void handleRoot(AsyncWebServerRequest *request)
{
    auto auth_ret = session_manager.getSessionInfo(request);
    Serial.println(bool(auth_ret));
    Serial.println(auth_ret._ip);
    for (auto &&i : auth_ret._mac.addr)
        Serial.printf("%u:", i);
    Serial.println();
    Serial.println(auth_ret._userID);
    request->send(200);
}

void handleAuth(AsyncWebServerRequest *request)
{
    session_manager.newSession(request, request->getParam("id")->value());
}

void handleLogoff(AsyncWebServerRequest *request)
{
    session_manager.removeSession(request);
}
