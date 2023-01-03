#include <ESPmDNS.h>
#include "src/ESP32SessionManager/ESPSessionManager.hpp"
#include <ESPAsyncWebServer.h>

AsyncWebServer server(80);

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
    Serial.println(WiFi.softAPIP());

    server.on("/", handleRoot);
    server.on("/auth", handleAuth);
    server.on("/logoff", handleLogoff);
    server.begin();

    MDNS.begin("test");
    MDNS.addService("http", "tcp", 80);
}

void loop()
{
}

void handleRoot(AsyncWebServerRequest *request)
{
    auto &auth_ret = cst::session_manager.getSessionInfo(request->client()->localIP(), request->client()->getRemoteAddress());
    log_d("%u", bool(auth_ret));
    log_d("%u", auth_ret._ip);
    log_d("MAC: %u:%u:%u:%u:%u:%u", auth_ret._mac.addr[0], auth_ret._mac.addr[1], auth_ret._mac.addr[2], auth_ret._mac.addr[3], auth_ret._mac.addr[4], auth_ret._mac.addr[5], auth_ret._mac.addr[6]);
    log_d("%s", auth_ret._userID);
    request->send(200, "application/json", JSON.stringify(cst::session_manager.toJSON()));
}

void handleAuth(AsyncWebServerRequest *request)
{
    cst::session_manager.newSession(request->client()->localIP(), request->client()->getRemoteAddress(), request->getParam("id")->value());
    request->redirect("/");
}

void handleLogoff(AsyncWebServerRequest *request)
{
    cst::session_manager.removeSession(request->client()->localIP(), request->client()->getRemoteAddress());
    request->redirect("/");
}
