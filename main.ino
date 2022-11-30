#define DEBUG_SESSIONMANAGER

#include "libs/ESPSessionManager.h"

AsyncWebServer server(80);

void handleRoot(AsyncWebServerRequest *request);
void handleAuth(AsyncWebServerRequest *request);

void setup()
{
    Serial.begin(115200);
    WiFi.disconnect(true, true);
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
    server.begin();
}

void loop()
{
}

void handleRoot(AsyncWebServerRequest *request)
{
    cst::session_manager.handleRequest(request);
    request->send(200);
}

void handleAuth(AsyncWebServerRequest *request)
{
    cst::session_manager.newSession(request, request->getParam("id")->value());
}
