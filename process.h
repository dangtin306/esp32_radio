#ifndef TEST_H
#define TEST_H

#include <PPP.h>
#include <HardwareSerial.h>
#include "Arduino.h"
#include "WiFi.h"
#include "Audio.h"

// Khai báo các hàm
bool sim_at_wait(String &response, unsigned long timeout = 2000);
bool sim_at_cmd(String cmd);
void resetSIM();
void onEvent(arduino_event_id_t event, arduino_event_info_t info);
void testClient(const char *host, uint16_t port);
bool initializeModem();
bool runSimATCommands();
void checkNetworkAndToggleLED();
void radio_start(Audio &audio);
void audio_info(const char *info);
void start_at_sim(int ledPin);
void start_ppp_sim(int ledPin);

#endif // TEST_H