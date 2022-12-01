#define DEBUG_SESSIONMANAGER

#include "ESPSessionManager.hpp"

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
            auto macCheck = std::equal(std::begin(thisSession->second._mac.addr), std::end(thisSession->second._mac.addr), std::begin(get_sta_mac(ip)->addr));
            if (macCheck)
                return thisSession->second;
            sta_sessions.erase(ip);
        }
        return emptySession;
    }

    SessionInfo_t ESPSessionManager::getSessionInfo(AsyncWebServerRequest *request)
    {
        if (ON_AP_FILTER(request))
            return handle_ap_ip(request->client()->getRemoteAddress());
        return handle_sta_ip(request->client()->getRemoteAddress());
    }

    void ESPSessionManager::newSession(AsyncWebServerRequest *request, const String &id)
    {
        if (ON_STA_FILTER(request))
            sta_sessions.emplace(request->client()->getRemoteAddress(), SessionInfo_t(request->client()->getRemoteAddress(), *get_sta_mac(request->client()->getRemoteAddress()), id));
        else
            ap_sessions.at(request->client()->getRemoteAddress())._userID = id;
    }

    void ESPSessionManager::removeSession(AsyncWebServerRequest *request)
    {
        if (ON_STA_FILTER(request))
            sta_sessions.erase(request->client()->getRemoteAddress());
        else
            ap_sessions.at(request->client()->getRemoteAddress())._userID = emptyString;
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
} // namespace cst