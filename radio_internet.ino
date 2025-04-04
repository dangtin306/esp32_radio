#include "test.h"

#define simSerial Serial2
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

#define I2S_DOUT 9
#define I2S_BCLK 3
#define I2S_LRC 1

#define LED_PIN 2

String get_device_id();
String send_device_id();

Audio audio;
unsigned long lastInfoTime = 0;
unsigned long infoCountTime = 0;
int infoCount = 0;
unsigned long tsInfoCountTime = 0;
int tsInfoCount = 0;
bool needDelay = false;
unsigned long delayStartTime = 0;
const unsigned long delayDuration = 150;
String device_id;

void setup() {
  delay(500);
  pinMode(MCU_SIM_EN_PIN, OUTPUT);
  digitalWrite(MCU_SIM_EN_PIN, HIGH);
  resetSIM();
  Serial.begin(115200);
  Serial.println("\n\n\n\n-----------------------\nSystem started!!!!");
  delay(1000);
  device_id = get_device_id();
  Serial.print("Device ID: ");
  Serial.println(device_id);
  delay(9000);
  start_at_sim(LED_PIN);  // Gọi hàm
  delay(300);
  Serial.println("\nketnoi!");
  Network.onEvent(onEvent);
  int attempts_2 = 0;
  bool modemAttached = false;
  while (attempts_2 < 7 && !modemAttached) {
    modemAttached = initializeModem();
    if (!modemAttached) {
      Serial.println("Modem not attached. Retrying...");
      attempts_2++;
      delay(2000);
    }
  }
  if (!modemAttached) {
    Serial.println("Modem failed to attach after 7 attempts.");
  }
  delay(100);
  start_ppp_sim(LED_PIN);
  delay(100);
  send_device_id(device_id);
  // Thiết lập MQTT sau khi kết nối mạng qua PPP
  setupMQTT();
  radio_start(audio);
}

void loop() {
  // Quản lý kết nối và xử lý tin nhắn MQTT
  loopMQTT();
  audio.loop();
  if (millis() - lastInfoTime > 6000) {
    Serial.println("No info received in 6 seconds. Restarting radio...");
    lastInfoTime = millis();
    radio_start(audio);
  }
  if (millis() - tsInfoCountTime > 9000 && tsInfoCount < 2) {
    Serial.println("Not enough '.aac' info received in 9 seconds. Restarting radio...");
    tsInfoCountTime = millis();
    tsInfoCount = 0;
    radio_start(audio);
  }
  if (needDelay) {
    unsigned long elapsed = millis() - delayStartTime;
    if (elapsed < delayDuration) {
      vTaskDelay((delayDuration - elapsed) / portTICK_PERIOD_MS);
    }
    needDelay = false;
  }
  vTaskDelay(3 / portTICK_PERIOD_MS);
}

void audio_info(const char *info) {
  lastInfoTime = millis();  // Cập nhật thời gian nhận thông báo cuối cùng
  Serial.print("info        ");
  Serial.println(info);
  String infoStr = String(info);

  // Xử lý các điều kiện để khởi động lại radio
  if (infoStr.indexOf("Request") != -1 && infoStr.indexOf("failed") != -1) {
    radio_start(audio);
    return;
  }
  if (infoStr.indexOf("End") != -1 && infoStr.indexOf("webstream") != -1) {
    radio_start(audio);
    return;
  }
  if (infoStr.indexOf("Stream") != -1 && infoStr.indexOf("lost") != -1) {
    radio_start(audio);
    return;
  }
  if (infoStr.indexOf("connect") != -1 && infoStr.indexOf("lost") != -1) {
    radio_start(audio);
    return;
  }
  if (infoStr.indexOf("framesize") != -1 && infoStr.indexOf("decoding") != -1 && infoStr.indexOf("again") != -1) {
    radio_start(audio);
    return;
  }
  if (infoStr.indexOf("Unexpected") != -1 && infoStr.indexOf("channel") != -1 && infoStr.indexOf("change") != -1) {
    delay(2000);
    radio_start(audio);
    return;
  }
  // Xử lý thông báo chứa "connect" và "m3u8"
  if (infoStr.indexOf("connect") != -1 && infoStr.indexOf("m3u8") != -1) {
    unsigned long currentTime = millis();
    if (currentTime - infoCountTime > 2000) {
      infoCount = 0;
      infoCountTime = currentTime;
    }
    infoCount++;
    if (infoCount > 1 && !needDelay) {
      needDelay = true;
      delayStartTime = currentTime;
    }
    if (infoCount > 2) {
      infoCount = 0;
      infoCountTime = currentTime;
      needDelay = false;
      radio_start(audio);
    }
  }

  // Xử lý thông báo chứa "connect" và ".aac"
  if (infoStr.indexOf("connect") != -1 && infoStr.indexOf(".aac") != -1) {
    unsigned long currentTime = millis();
    if (currentTime - tsInfoCountTime > 7000) {
      tsInfoCount = 0;
      tsInfoCountTime = currentTime;
    }
    tsInfoCount++;
  }
}