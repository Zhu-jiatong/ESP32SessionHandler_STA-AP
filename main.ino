#include <ESPAsyncWebServer.h>
#include <lwip/etharp.h>
#include <WiFi.h>

AsyncWebServer server(80);

ip4_addr_t targetIP;
IPAddress subnet, localIP, gwip, IPtoCheck(172, 20, 10, 5);
uint32_t subnetSize;

void handleRequest(AsyncWebServerRequest *request);
void authRequest(AsyncWebServerRequest *request);

void setup()
{
    Serial.begin(115200);
    WiFi.begin("brenda iPad Mini 3 (3)", "2fkmby2spezi8");
    while (WiFi.status() != WL_CONNECTED)
    {
        Serial.println("Connecting");
        delay(500);
    }
    Serial.printf("Connected to %s", WiFi.SSID());
    Serial.println(WiFi.localIP());
    /*     subnet = WiFi.subnetMask();
        localIP = WiFi.localIP();
        gwip = WiFi.gatewayIP();
        subnetSize = (256 - subnet[0]) * (256 - subnet[1]) * (256 - subnet[2]) * (256 - subnet[3]) - 2;
     */
    server.on("/", handleRequest);
    server.begin();
}

void loop()
{
}

void authRequest(AsyncWebServerRequest *request)
{
    ip4_addr requestIP;
    requestIP.addr = request->client()->getRemoteAddress();
    Serial.println(requestIP.addr);
    eth_addr *ret_eth_addr = nullptr;
    ip4_addr const *ret_ip_addr = nullptr;
    etharp_request(netif_default, &requestIP);
    etharp_find_addr(netif_default, &requestIP, &ret_eth_addr, &ret_ip_addr);
    Serial.printf("mac: %02x:%02x:%02x:%02x:%02x:%02x\n\n", ret_eth_addr->addr[0], ret_eth_addr->addr[1], ret_eth_addr->addr[2], ret_eth_addr->addr[3], ret_eth_addr->addr[4], ret_eth_addr->addr[5]);
}

void handleRequest(AsyncWebServerRequest *request)
{
    authRequest(request);
    request->send(200);
}

void trash()
{ /*     IPAddress startIP = localIP;
startIP[0] &= subnet[0];
startIP[1] &= subnet[1];
startIP[2] &= subnet[2];
startIP[3] &= subnet[3];
startIP[3] += 1;
*/
    /*     for (int i = 0; i < subnetSize; i++)
     {
         IPAddress IPtoCheck = startIP;
         IPtoCheck[0] += (i / 255 / 255 / 255) % 256;
         IPtoCheck[1] += (i / 255 / 255) % 256;
         IPtoCheck[2] += (i / 255) % 256;
         IPtoCheck[3] += i % 256;
         if (IPtoCheck == localIP || IPtoCheck == gwip)
             continue;

  */
    ip4_addr_t toProbe;
    IP4_ADDR(&toProbe, IPtoCheck[0], IPtoCheck[1], IPtoCheck[2], IPtoCheck[3]);

    etharp_request(netif_default, &toProbe);
    Serial.printf("Scanning for: %s\n", IPtoCheck.toString().c_str());
    // delay(20);
    //}
    // delay(2000);
    /* for (int i = 0; i < subnetSize; i++)
    {
        IPAddress IPtoCheck = startIP;
        IPtoCheck[0] += (i / 255 / 255 / 255) % 256;
        IPtoCheck[1] += (i / 255 / 255) % 256;
        IPtoCheck[2] += (i / 255) % 256;
        IPtoCheck[3] += i % 256;
        if (IPtoCheck == localIP || IPtoCheck == gwip)
            continue;

     */
    ip4_addr_t toCheck;
    IP4_ADDR(&toCheck, IPtoCheck[0], IPtoCheck[1], IPtoCheck[2], IPtoCheck[3]);

    eth_addr *ret_eth_addr = NULL;
    ip4_addr const *ret_ip_addr = NULL;

    int arp_find = etharp_find_addr(netif_default, &toCheck, &ret_eth_addr, &ret_ip_addr);

    Serial.printf("Lookup: %s | %d\n", IPtoCheck.toString().c_str(), arp_find);

    if (arp_find != -1 && ret_eth_addr /*  && (Ping.ping(IPtoCheck, 1) || Ping.ping(IPtoCheck, 1)) */)
    {
        Serial.printf("mac: \"%02x:%02x:%02x:%02x:%02x:%02x\"", ret_eth_addr->addr[0], ret_eth_addr->addr[1], ret_eth_addr->addr[2], ret_eth_addr->addr[3], ret_eth_addr->addr[4], ret_eth_addr->addr[5]);
    }
    // delay(20);
    //}
}