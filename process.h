#ifndef TEST_H
#define TEST_H

#include "Arduino.h"
#include <PPP.h>
#include <HardwareSerial.h>
#include "WiFi.h"
#include <ETH.h>
#include <SPI.h>
#include <ArduinoJson.h>
#include "Audio.h"

extern bool eth_connected;

// audio GPIO
const int I2S_DOUT = 9;
const int I2S_BCLK = 3;
const int I2S_LRC = 1;
const int LED_PIN = 2;
// Khai báo các hàm
bool sim_at_wait(String &response, unsigned long timeout = 2000);
bool sim_at_cmd(String cmd);
void resetSIM();
void onEvent_sim(arduino_event_id_t event, arduino_event_info_t info);
void onEvent_lan(arduino_event_id_t event, arduino_event_info_t info);
void testClient(const char *host, uint16_t port);
bool initializeModem();
bool runSimATCommands();
void checkNetworkAndToggleLED();
// Sửa khai báo hàm radio_restart để nhận thêm tham số live_url_audio dạng String
// void radio_restart(Audio &audio, String live_url_audio);
void audio_info(const char *info);
void start_at_sim(int ledPin);
void start_ppp_sim(int ledPin);

#endif // TEST_H