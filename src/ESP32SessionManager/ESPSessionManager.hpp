#if !defined(ESPSESSIONMANAGER_h)
#define ESPSESSIONMANAGER_h

#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <unordered_map>
#include <lwip/etharp.h>
#include <esp_wifi.h>

struct SessionInfo_t
{
    uint32_t _ip = 0;
    eth_addr _mac{};
    String _userID;

    operator bool() const { return _userID.length(); }

    SessionInfo_t(const uint32_t &ip, const eth_addr mac, const String &id) : _ip(ip), _mac(mac), _userID(id)
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

    SessionInfo_t(){};
#if defined(DEBUG_SESSIONMANAGER)

    ~SessionInfo_t()
    {
        Serial.println("Session removed:");
        Serial.printf("IP: %u\n", _ip);
        Serial.print("MAC: ");
        for (auto &&i : _mac.addr)
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

        void onStaConnect(const uint32_t &ip);         // register new station IP & MAC address once device connects to AP
        void onStaDisconnect(const uint8_t (&mac)[6]); // remove session once a AP station disconnects from Wi-Fi

    public:
        SessionInfo_t getSessionInfo(AsyncWebServerRequest *request);
        void newSession(AsyncWebServerRequest *request, const String &id);
        void removeSession(AsyncWebServerRequest *request);
        eth_addr *get_sta_mac(const uint32_t &ip);
        SessionInfo_t handle_sta_ip(const uint32_t &ip);
        SessionInfo_t handle_ap_ip(const uint32_t &ip);

        ESPSessionManager(); // register Wi-Fi event handlers for connection and disconnection

        SessionInfo_t emptySession;
    };
} // namespace cst

#endif // ESPSESSIONMANAGER_h
