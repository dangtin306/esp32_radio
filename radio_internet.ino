#include "process.h"
#include <Preferences.h>

Preferences preferences;

// Định nghĩa dưới dạng String toàn cục
String live_url_audio;
String device_id;
int volume;

#define I2S_DOUT 9
#define I2S_BCLK 3
#define I2S_LRC 1

#define LED_PIN 2

void connect_sim();
void connect_wifi();
void connect_lan();
String get_device_id();
String send_device_id();
String get_link_live();
String radio_restart();
String setVolume();
String loopMQTT();

Audio audio;
unsigned long lastInfoTime = 0;
unsigned long infoCountTime = 0;
int infoCount = 0;
unsigned long tsInfoCountTime = 0;
int tsInfoCount = 0;
bool needDelay = false;
unsigned long delayStartTime = 0;
const unsigned long delayDuration = 150;

void setup()
{
  delay(500);
  Serial.begin(115200);
  Serial.println("\n\n\n\n-----------------------\nSystem started!!!!");
  delay(200);
  device_id = get_device_id();
  Serial.print("Device ID: ");
  Serial.println(device_id);
  connect_sim();
  delay(700);
  send_device_id(device_id);
  get_link_live(device_id);
  // Setup MQTT after connecting to the network via PPP
  setupMQTT();
  radio_restart(audio);
}

void loop()
{
  // Manage MQTT connection and process MQTT messages
  loopMQTT();
  audio.loop();
  if (millis() - lastInfoTime > 6000)
  {
    Serial.println("No info received in 6 seconds. Restarting radio...");
    lastInfoTime = millis();
    radio_restart(audio);
  }
  if (millis() - tsInfoCountTime > 9000 && tsInfoCount < 2)
  {
    Serial.println("Not enough '.aac' info received in 9 seconds. Restarting radio...");
    tsInfoCountTime = millis();
    tsInfoCount = 0;
    radio_restart(audio);
  }
  if (needDelay)
  {
    unsigned long elapsed = millis() - delayStartTime;
    if (elapsed < delayDuration)
    {
      vTaskDelay((delayDuration - elapsed) / portTICK_PERIOD_MS);
    }
    needDelay = false;
  }
  vTaskDelay(3 / portTICK_PERIOD_MS);
}