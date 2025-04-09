#include "WiFi.h"
#include <ETH.h>
#include <SPI.h>

//  WiFi info
const char *ssid = "TECOMNEWS";
const char *password = "hictecom31102009a@";

//  Sim info
#define MCU_SIM_BAUDRATE 115200
#define MCU_SIM_TX_PIN 17
#define MCU_SIM_RX_PIN 18
#define MCU_SIM_EN_PIN 4

#define PPP_MODEM_APN "internet"
#define PPP_MODEM_PIN "0000"
#define PPP_MODEM_RST -1
#define PPP_MODEM_RST_LOW false
#define PPP_MODEM_RST_DELAY 200
#define PPP_MODEM_TX 17
#define PPP_MODEM_RX 18
#define PPP_MODEM_RTS -1
#define PPP_MODEM_CTS -1
#define PPP_MODEM_FC ESP_MODEM_FLOW_CONTROL_NONE
#define PPP_MODEM_MODEL PPP_MODEM_SIM7600

//  Lan info
#define ETH_PHY_ADDR 1
#define ETH_PHY_CS 14
#define ETH_PHY_IRQ 2
#define ETH_PHY_RST -1
#define ETH_SPI_SCK 13
#define ETH_SPI_MISO 11
#define ETH_SPI_MOSI 12
#define I2S_DOUT 9
#define I2S_BCLK 3
#define I2S_LRC 1

void connect_sim()
{
  resetSIM();
  delay(9000);
  start_at_sim(LED_PIN); // Call the function
  delay(300);
  Serial.println("\nConnected sim module!");
  Network.onEvent(onEvent);
  int attempts_2 = 0;
  bool modemAttached = false;
  while (attempts_2 < 7 && !modemAttached)
  {
    modemAttached = initializeModem();
    if (!modemAttached)
    {
      Serial.println("Modem not attached. Retrying...");
      attempts_2++;
      delay(2000);
    }
  }
  if (!modemAttached)
  {
    Serial.println("Modem failed to attach after 7 attempts.");
  }
  delay(100);
  start_ppp_sim(LED_PIN);
}

void connect_wifi()
{
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  WiFi.setTxPower(WIFI_POWER_8_5dBm);
  Serial.print("Đang kết nối WiFi");
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi đã kết nối!");
  Serial.print("Địa chỉ IP: ");
  Serial.println(WiFi.localIP());
}

static bool eth_connected = false;

void onEvent(arduino_event_id_t event, arduino_event_info_t info) {
  switch (event) {
    case ARDUINO_EVENT_ETH_START:
      Serial.println("ETH Started");
      ETH.setHostname("esp32-eth0");  //set eth hostname here
      break;
    case ARDUINO_EVENT_ETH_CONNECTED: Serial.println("ETH Connected"); break;
    case ARDUINO_EVENT_ETH_GOT_IP:
      Serial.printf("ETH Got IP: '%s'\n", esp_netif_get_desc(info.got_ip.esp_netif));
      Serial.println(ETH);
      eth_connected = true;
      break;
    case ARDUINO_EVENT_ETH_LOST_IP:
      Serial.println("ETH Lost IP");
      eth_connected = false;
      break;
    case ARDUINO_EVENT_ETH_DISCONNECTED:
      Serial.println("ETH Disconnected");
      eth_connected = false;
      break;
    case ARDUINO_EVENT_ETH_STOP:
      Serial.println("ETH Stopped");
      eth_connected = false;
      break;
    default: break;
  }
}


void connect_lan()
{
  Network.onEvent(onEvent);
  SPI.begin(ETH_SPI_SCK, ETH_SPI_MISO, ETH_SPI_MOSI);
  ETH.begin(ETH_PHY_TYPE, ETH_PHY_ADDR, ETH_PHY_CS, ETH_PHY_IRQ, ETH_PHY_RST, SPI);
  while (!eth_connected) delay(100);
}

