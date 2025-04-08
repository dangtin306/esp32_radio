#include "process.h"


bool sim_at_wait(String &response, unsigned long timeout) {
  unsigned long start = millis();
  response = "";
  while (millis() - start < timeout) {
    while (Serial2.available()) {
      response += char(Serial2.read());
    }
    if (response.indexOf("OK") != -1) {
      return true;
    }
  }
  return false;
}

bool sim_at_cmd(String cmd) {
  Serial2.println(cmd);
  String resp;
  bool success = sim_at_wait(resp);
  Serial.print("Response for ");
  Serial.print(cmd);
  Serial.print(": ");
  Serial.println(resp);
  return success;
}

void resetSIM() {
  delay(500);
  digitalWrite(4, HIGH);  // MCU_SIM_EN_PIN được định nghĩa trong .ino
  delay(500);
  digitalWrite(4, LOW);
  delay(500);
  digitalWrite(4, HIGH);
  delay(500);
}

void onEvent(arduino_event_id_t event, arduino_event_info_t info) {
  switch (event) {
    case ARDUINO_EVENT_PPP_START: Serial.println("PPP Started"); break;
    case ARDUINO_EVENT_PPP_CONNECTED: Serial.println("PPP Connected"); break;
    case ARDUINO_EVENT_PPP_GOT_IP: Serial.println("PPP Got IP"); break;
    case ARDUINO_EVENT_PPP_LOST_IP: Serial.println("PPP Lost IP"); break;
    case ARDUINO_EVENT_PPP_DISCONNECTED: Serial.println("PPP Disconnected"); break;
    case ARDUINO_EVENT_PPP_STOP: Serial.println("PPP Stopped"); break;
    default: break;
  }
}

void testClient(const char *host, uint16_t port) {
  NetworkClient client;
  if (!client.connect(host, port)) {
    Serial.println("Connection Failed");
    return;
  }
  client.printf("GET / HTTP/1.1\r\nHost: %s\r\n\r\n", host);
  while (client.connected() && !client.available())
    ;
  while (client.available()) {
    client.read();
  }
  Serial.println("Connection Success ok");
  client.stop();
}

bool initializeModem() {
  PPP.setApn("internet");                                    // PPP_MODEM_APN từ .ino
  PPP.setPin("0000");                                        // PPP_MODEM_PIN từ .ino
  PPP.setResetPin(-1, false, 200);                           // PPP_MODEM_RST, RST_LOW, RST_DELAY từ .ino
  PPP.setPins(17, 18, -1, -1, ESP_MODEM_FLOW_CONTROL_NONE);  // TX, RX, RTS, CTS, FC từ .ino
  delay(2000);
  Serial.println("\nStarting the modem. It might take a while!");
  PPP.begin(PPP_MODEM_SIM7600);  // PPP_MODEM_MODEL từ .ino
  delay(1000);
  Serial.print("Manufacturer: ");
  Serial.println(PPP.cmd("AT+CGMI", 10000));
  Serial.print("Model: ");
  Serial.println(PPP.moduleName());
  Serial.print("IMEI: ");
  Serial.println(PPP.IMEI());
  bool attached = PPP.attached();
  Serial.print("attached: ");
  Serial.println(attached);
  delay(500);
  if (!attached) {
    PPP.end();
    Serial.println("PPP disconnected.");
    delay(500);
  }
  return attached;
}

bool runSimATCommands() {
  Serial2.begin(115200, SERIAL_8N1, 18, 17);  // Baudrate, RX, TX từ .ino
  delay(1000);
  Serial.println("Sending AT commands sequence...");
  Serial.println("AT");
  if (!sim_at_cmd("AT")) {
    Serial.println("No response for AT command.");
    Serial2.end();
    return false;
  }
  delay(200);
  Serial.println("ATI");
  sim_at_cmd("ATI");
  delay(200);
  Serial.println("AT+IPR=?");
  sim_at_cmd("AT+IPR=?");
  delay(200);
  Serial.println("AT+IPR?");
  sim_at_cmd("AT+IPR?");
  delay(200);
  for (int i = 0; i < 8; i++) {
    Serial.println("AT+IPR=115200");
    if (sim_at_cmd("AT+IPR=115200")) {
      Serial.println("AT+IPR=115200 command succeeded. Exiting loop.");
      Serial2.end();
      delay(500);
      return true;
    }
    delay(500);
  }
  Serial.println("AT+IPR?");
  sim_at_cmd("AT+IPR?");
  delay(300);
  Serial2.end();
  delay(300);
  return false;
}

void checkNetworkAndToggleLED() {
  static unsigned long lastCheckTime = 0;
  unsigned long currentTime = millis();
  if (currentTime - lastCheckTime >= 10000) {
    Serial.println("Thử lại mạng!");
    testClient("google.com", 80);
    lastCheckTime = currentTime;
  }
}


void start_at_sim(int ledPin) {
  int attempts_1 = 0;
  bool atSuccess = false;
  while (attempts_1 < 5 && !atSuccess) {
    Serial.print("Attempt ");
    Serial.println(attempts_1 + 1);
    atSuccess = runSimATCommands();
    if (!atSuccess) {
      attempts_1++;
      Serial.println("AT command sequence failed, retrying...");
      delay(2000);
    }
  }
  if (!atSuccess) {
    Serial.println("AT command sequence failed after 5 attempts.");
  } else {
    Serial.println("AT command sequence succeeded.");
  }
  delay(300);
  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, HIGH);
}

void start_ppp_sim(int ledPin) {
  PPP.setBaudrate(921600);
  delay(300);
  Serial.println("\nketnoi internet!");
  bool attached = PPP.attached();
  Serial.print("attached: ");
  Serial.println(attached);

  if (!attached) {
    int i = 0;
    unsigned int s = millis();
    Serial.print("Waiting to connect to network");
    while (!attached && ((++i) < 29)) {
      Serial.print(".");
      delay(1000);
      attached = PPP.attached();
      digitalWrite(ledPin, !digitalRead(ledPin));
    }
    Serial.print((millis() - s) / 1000.0, 1);
    Serial.println("s");
    attached = PPP.attached();
  }

  Serial.print("Attached: ");
  Serial.println(attached);
  Serial.print("State: ");
  Serial.println(PPP.radioState());

  if (attached) {
    Serial.print("Operator: ");
    Serial.println(PPP.operatorName());
    Serial.print("IMSI: ");
    Serial.println(PPP.IMSI());
    Serial.print("RSSI: ");
    Serial.println(PPP.RSSI());
    int ber = PPP.BER();
    if (ber > 0) {
      Serial.print("BER: ");
      Serial.println(ber);
      Serial.print("NetMode: ");
      Serial.println(PPP.networkMode());
    }
    Serial.println("Switching to data mode...");
    PPP.mode(ESP_MODEM_MODE_CMUX);
    if (!PPP.waitStatusBits(ESP_NETIF_CONNECTED_BIT, 1000)) {
      Serial.println("Failed to connect to internet!");
    } else {
      Serial.println("Connected to internet!");
    }
  } else {
    Serial.println("Failed to connect to network!");
  }

  delay(200);
  checkNetworkAndToggleLED();
}