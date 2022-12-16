#if !defined(ESPSESSIONMANAGER_h)
#define ESPSESSIONMANAGER_h

#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <unordered_map>
#include <lwip/etharp.h>
#include <esp_wifi.h>
#include <Arduino_JSON.h>

struct SessionInfo_t
{
    uint32_t _ip = 0;
    eth_addr _mac{};
    String _userID;

    JSONVar toJSON();
    operator bool() const { return _userID.length(); }
    SessionInfo_t(const uint32_t &ip, const eth_addr mac, const String &id);
    SessionInfo_t(){};
    ~SessionInfo_t();
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

    extern ESPSessionManager session_manager;
} // namespace cst

#endif // ESPSESSIONMANAGER_h
