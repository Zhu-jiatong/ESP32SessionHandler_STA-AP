#if !defined(ESPSESSIONMANAGER_h)
#define ESPSESSIONMANAGER_h

#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <unordered_map>
#include <lwip/etharp.h>
#include <esp_wifi.h>

struct SessionInfo_t
{
    uint32_t _ip;
    std::array<uint8_t, 6> _mac;
    String _userID;

    SessionInfo_t(const uint32_t &ip, const uint8_t (&mac)[6], const String &id) : _ip(ip), _userID(id)
    {
        std::move(std::begin(mac), std::end(mac), _mac.begin());

#if defined(DEBUG_SESSIONMANAGER)

        Serial.println("Session created");
        Serial.printf("IP: %u\n", _ip);
        Serial.print("MAC: ");
        for (auto &&i : _mac)
            Serial.printf("%u:", i);
        Serial.printf("\nID: %s\n\n", _userID);

#endif // DEBUG_SESSIONMANAGER
    }

#if defined(DEBUG_SESSIONMANAGER)

    ~SessionInfo_t()
    {
        Serial.println("Session removed:");
        Serial.printf("IP: %u\n", _ip);
        Serial.print("MAC: ");
        for (auto &&i : _mac)
            Serial.printf("%u:", i);
        Serial.printf("\nID: %s\n\n", _userID);
    }

#endif // DEBUG_SESSIONMANAGER
};

namespace cst
{
    class ESPSessionManager
    {
    private:
        std::unordered_map<uint32_t, SessionInfo_t> sta_sessions, ap_sessions;

        void onStaConnect(WiFiEvent_t event, WiFiEventInfo_t info);
        void onStaDisconnect(WiFiEvent_t event, WiFiEventInfo_t info);

    public:
        bool handleRequest(AsyncWebServerRequest *request);
        void newSession(AsyncWebServerRequest *request, const String &id);
        eth_addr *get_sta_mac(const uint32_t &ip);
        eth_addr *get_ap_mac(const uint32_t &ip);
        bool handle_sta_ip(const uint32_t &ip);
        bool handle_ap_ip(const uint32_t &ip);

        ESPSessionManager();
    } session_manager;

    eth_addr *ESPSessionManager::get_sta_mac(const uint32_t &ip)
    {
        ip4_addr requestIP{ip};
        eth_addr *ret_eth_addr = nullptr;
        ip4_addr const *ret_ip_addr = nullptr;
        etharp_request(netif_default, &requestIP);
        etharp_find_addr(netif_default, &requestIP, &ret_eth_addr, &ret_ip_addr);
        return ret_eth_addr;
    }

    eth_addr *ESPSessionManager::get_ap_mac(const uint32_t &ip) {}

    bool ESPSessionManager::handle_ap_ip(const uint32_t &ip) {}

    bool ESPSessionManager::handle_sta_ip(const uint32_t &ip)
    {
        auto thisSession = sta_sessions.find(ip);
        if (thisSession != sta_sessions.end())
        {
            auto macCheck = std::equal(thisSession->second._mac.begin(), thisSession->second._mac.end(), std::begin(get_sta_mac(ip)->addr));
            if (macCheck)
                return true;
            sta_sessions.erase(ip);
        }
        return false;
    }

    bool ESPSessionManager::handleRequest(AsyncWebServerRequest *request)
    {
        if (ON_AP_FILTER(request))
            return handle_ap_ip(request->client()->getRemoteAddress());
        return handle_sta_ip(request->client()->getRemoteAddress());
    }

    void ESPSessionManager::newSession(AsyncWebServerRequest *request, const String &id)
    {
        sta_sessions.emplace(request->client()->getRemoteAddress(), SessionInfo_t(request->client()->getRemoteAddress(), get_sta_mac(request->client()->getRemoteAddress())->addr, id));
    }

    ESPSessionManager::ESPSessionManager()
    {
        WiFi.onEvent([&](WiFiEvent_t event, WiFiEventInfo_t info)
                     { onStaConnect(event, info); },
                     ARDUINO_EVENT_WIFI_AP_STAIPASSIGNED);
        WiFi.onEvent([&](WiFiEvent_t event, WiFiEventInfo_t info)
                     { onStaDisconnect(event, info); },
                     ARDUINO_EVENT_WIFI_AP_STADISCONNECTED);
    }

    void ESPSessionManager::onStaConnect(WiFiEvent_t event, WiFiEventInfo_t info)
    {
        wifi_sta_list_t wifi_sta_list;
        esp_netif_sta_list_t netif_sta_list;

        esp_wifi_ap_get_sta_list(&wifi_sta_list);
        esp_netif_get_sta_list(&wifi_sta_list, &netif_sta_list);
        for (size_t i = 0; i < netif_sta_list.num; ++i)
        {
            auto thisSession = ap_sessions.find(netif_sta_list.sta[i].ip.addr);
            if (thisSession != ap_sessions.end())
                if (std::equal(thisSession->second._mac.begin(), thisSession->second._mac.end(), std::begin(netif_sta_list.sta[i].mac)))
                    continue;
                else
                    ap_sessions.erase(netif_sta_list.sta[i].ip.addr);
            ap_sessions.emplace(netif_sta_list.sta[i].ip.addr, SessionInfo_t(netif_sta_list.sta[i].ip.addr, netif_sta_list.sta[i].mac, emptyString));
        }
    }

    void ESPSessionManager::onStaDisconnect(WiFiEvent_t event, WiFiEventInfo_t info)
    {
        for (auto client = ap_sessions.begin(); client != ap_sessions.end(); ++client)
            if (std::equal(client->second._mac.begin(), client->second._mac.end(), std::begin(info.wifi_ap_stadisconnected.mac)))
                ap_sessions.erase(client);
    }
} // namespace cst

#endif // ESPSESSIONMANAGER_h
