#include "WiFi.h"

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