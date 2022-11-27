#include <ESPAsyncWebServer.h>
#include <lwip/etharp.h>
#include <WiFi.h>
#include <unordered_map>

AsyncWebServer server(80);

struct SessionInfo_t
{
    uint32_t _ip_addr;
    std::array<uint8_t, 6> _mac_addr;
    String _user_id;

    SessionInfo_t(const uint32_t &ip, const uint8_t (&mac)[6], const String &id) : _ip_addr(ip), _user_id(id)
    {
        std::move(std::begin(mac), std::end(mac), std::begin(_mac_addr));
        Serial.println("Session created");
        Serial.printf("IP: %u\n", _ip_addr);
        Serial.print("MAC: ");
        for (auto &&i : _mac_addr)
            Serial.printf("%u:", i);
        Serial.printf("\nID: %s\n\n", _user_id);
    }
    ~SessionInfo_t()
    {
        Serial.println("Session removed:");
        Serial.printf("IP: %u\n", _ip_addr);
        Serial.print("MAC: ");
        for (auto &&i : _mac_addr)
            Serial.printf("%u:", i);
        Serial.printf("\nID: %s\n\n", _user_id);
    }
};

std::unordered_map<uint32_t, SessionInfo_t> sessions;

void handleRequest(AsyncWebServerRequest *request);
void authRequest(const uint32_t &IPAddr);

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
    server.on("/", handleRequest);
    server.begin();
}

void loop()
{
}

void authRequest(const uint32_t &IPAddr)
{
    ip4_addr requestIP{IPAddr};
    Serial.println(requestIP.addr);
    eth_addr *ret_eth_addr = nullptr;
    ip4_addr const *ret_ip_addr = nullptr;
    etharp_request(netif_default, &requestIP);
    etharp_find_addr(netif_default, &requestIP, &ret_eth_addr, &ret_ip_addr);
    auto existSession = sessions.find(IPAddr);
    if (existSession != sessions.end())
    {
        if (std::equal(existSession->second._mac_addr.begin(), existSession->second._mac_addr.end(), std::begin(ret_eth_addr->addr)))
        {
            Serial.println("Auth success");
            Serial.printf("IP: %u\n", IPAddr);
            Serial.print("MAC: ");
            for (auto &&i : sessions.at(IPAddr)._mac_addr)
                Serial.printf("%u:", i);
            Serial.printf("\nID: %s\n\n", sessions.at(IPAddr)._user_id);
        }
        else
        {
            sessions.erase(IPAddr);
        }
    }
    else
    {
        sessions.emplace(IPAddr, SessionInfo_t(IPAddr, ret_eth_addr->addr, emptyString));
    }
}

void handleRequest(AsyncWebServerRequest *request)
{
    authRequest(request->client()->getRemoteAddress());
    request->send(200);
}
