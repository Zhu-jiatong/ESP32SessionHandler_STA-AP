#define DEBUG_SESSIONMANAGER

#include "ESPSessionManager.hpp"

JSONVar SessionInfo_t::toJSON()
{
    JSONVar session_JSON;
    session_JSON["ip"] = _ip;
    session_JSON["id"] = _userID;
    for (uint8_t i = 0, *it = std::begin(_mac.addr); it != std::end(_mac.addr); ++it, ++i)
        session_JSON["mac"][i] = *it;

    return session_JSON;
}

SessionInfo_t::SessionInfo_t(const uint32_t &ip, const eth_addr mac, const String &id) : _ip(ip), _mac(mac), _userID(id)
{
#if defined(DEBUG_SESSIONMANAGER)

    Serial.println("Session created");
    Serial.printf("IP: %u\n", _ip);
    Serial.print("MAC: ");
    for (auto &&i : _mac.addr)
        Serial.printf("%u:", i);
    Serial.printf("\nID: %s\n\n", _userID);

#endif // DEBUG_SESSIONMANAGER
}

SessionInfo_t::~SessionInfo_t()
{
#if defined(DEBUG_SESSIONMANAGER)

    Serial.println("Session removed:");
    Serial.printf("IP: %u\n", _ip);
    Serial.print("MAC: ");
    for (auto &&i : _mac.addr)
        Serial.printf("%u:", i);
    Serial.printf("\nID: %s\n\n", _userID);

#endif // DEBUG_SESSIONMANAGER
}

namespace cst
{
    eth_addr *ESPSessionManager::get_sta_mac(const uint32_t &ip)
    {
        ip4_addr requestIP{ip};
        eth_addr *ret_eth_addr = nullptr;
        ip4_addr const *ret_ip_addr = nullptr;
        etharp_request(netif_default, &requestIP);
        etharp_find_addr(netif_default, &requestIP, &ret_eth_addr, &ret_ip_addr);
        return ret_eth_addr;
    }

    SessionInfo_t ESPSessionManager::handle_ap_ip(const uint32_t &ip)
    {
        auto thisSession = ap_sessions.find(ip);
        return (thisSession != ap_sessions.end()) ? thisSession->second : emptySession;
    }

    SessionInfo_t ESPSessionManager::handle_sta_ip(const uint32_t &ip)
    {
        auto thisSession = sta_sessions.find(ip);
        if (thisSession != sta_sessions.end())
        {
            if (std::equal(std::begin(thisSession->second._mac.addr), std::end(thisSession->second._mac.addr), std::begin(get_sta_mac(ip)->addr)))
                return thisSession->second;
            sta_sessions.erase(ip);
        }
        return emptySession;
    }

    SessionInfo_t ESPSessionManager::getSessionInfo(const IPAddress &localIP, const uint32_t &remoteAddr)
    {
        if (isAP(localIP))
            return handle_ap_ip(remoteAddr);
        return handle_sta_ip(remoteAddr);
    }

    void ESPSessionManager::newSession(const IPAddress &localIP, const uint32_t &remoteAddr, const String &id)
    {
        if (isSTA(localIP))
            sta_sessions.emplace(remoteAddr, SessionInfo_t(remoteAddr, *get_sta_mac(remoteAddr), id));
        else
            ap_sessions.at(remoteAddr)._userID = id;
    }

    void ESPSessionManager::removeSession(const IPAddress &localIP, const uint32_t &remoteAddr)
    {
        if (isSTA(localIP))
            sta_sessions.erase(remoteAddr);
        else
            ap_sessions.at(remoteAddr)._userID = emptyString;
    }

    ESPSessionManager::ESPSessionManager()
    { // register Wi-Fi event handlers for connection and disconnection
        WiFi.onEvent([&](WiFiEvent_t event, WiFiEventInfo_t info)
                     { onStaConnect(info.wifi_ap_staipassigned.ip.addr); },
                     ARDUINO_EVENT_WIFI_AP_STAIPASSIGNED);
        WiFi.onEvent([&](WiFiEvent_t event, WiFiEventInfo_t info)
                     { onStaDisconnect(info.wifi_ap_stadisconnected.mac); },
                     ARDUINO_EVENT_WIFI_AP_STADISCONNECTED);
    }

    void ESPSessionManager::onStaConnect(const uint32_t &ip)
    { // register new station IP & MAC address once device connects to AP
        wifi_sta_list_t wifi_sta_list;
        esp_netif_sta_list_t netif_sta_list;
        esp_wifi_ap_get_sta_list(&wifi_sta_list);
        esp_netif_get_sta_list(&wifi_sta_list, &netif_sta_list);

        /* esp_netif_sta_info_t new_sta_info;
        for (auto &&sta_info : netif_sta_list.sta)
            if (sta_info.ip.addr == ip)
            {
                new_sta_info = sta_info;
                break;
            } */

        auto new_sta_info = std::find_if(std::begin(netif_sta_list.sta), std::end(netif_sta_list.sta),
                                         [&](esp_netif_sta_info_t temp_sta_info)
                                         { return temp_sta_info.ip.addr == ip; });
        Serial.println(new_sta_info->ip.addr);
        ap_sessions.erase(new_sta_info->ip.addr);
        eth_addr temp;
        std::move(std::begin(new_sta_info->mac), std::end(new_sta_info->mac), std::begin(temp.addr));
        ap_sessions.emplace(new_sta_info->ip.addr, SessionInfo_t(new_sta_info->ip.addr, temp, emptyString));
    }

    void ESPSessionManager::onStaDisconnect(const uint8_t (&mac)[6])
    { // remove session once a AP station disconnects from Wi-Fi
        for (auto client = ap_sessions.begin(); client != ap_sessions.end(); ++client)
            if (std::equal(std::begin(client->second._mac.addr), std::end(client->second._mac.addr), std::begin(mac)))
                ap_sessions.erase(client);
    }

    inline bool ESPSessionManager::isAP(const IPAddress &localIP)
    {
        return WiFi.localIP() != localIP;
    }

    inline bool ESPSessionManager::isSTA(const IPAddress &localIP)
    {
        return WiFi.localIP() == localIP;
    }

    ESPSessionManager session_manager;
} // namespace cst