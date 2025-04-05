#include <PubSubClient.h>
#include "test.h" // Giả sử test.h chứa các định nghĩa cần thiết

// Thông tin MQTT broker
const char* mqtt_server = "vip.tecom.pro";
const int mqtt_port = 1883;

// Khởi tạo client MQTT
WiFiClient espClient;
PubSubClient client(espClient);

// Hàm callback xử lý tin nhắn MQTT
void mqttCallback(char* topic, unsigned char* payload, unsigned int length) {
  String message;
  for (unsigned int i = 0; i < length; i++) {
    message += (char)payload[i];
  }
  Serial.printf("Nhận được tin nhắn từ topic '%s': %s\n", topic, message.c_str());
  
  // Nếu tin nhắn là "reset", khởi động lại thiết bị ESP32
  if (message.equals("reset")) {
    Serial.println("Nhận lệnh reset. Khởi động lại thiết bị...");
    ESP.restart();
  }
  
  // Xây dựng topic gửi phản hồi động: "device/send/<device_id>"
  String publish_topic = "device/send/" + device_id;
  String response = "Đã nhận: " + message;
  client.publish(publish_topic.c_str(), response.c_str());
  Serial.printf("Đã gửi phản hồi đến topic '%s': %s\n", publish_topic.c_str(), response.c_str());
}

// Hàm kết nối lại MQTT
void reconnect() {
  while (!client.connected()) {
    Serial.print("Kết nối MQTT...");
    String clientId = "ESP32Client-";
    clientId += String(random(0xffff), HEX);
    if (client.connect(clientId.c_str())) {
      Serial.println("Kết nối thành công!");
      
      // Xây dựng topic subscribe động: "server/send/<device_id>"
      String subscribe_topic = "server/send/" + device_id;
      client.subscribe(subscribe_topic.c_str());
      
      // Gửi thông báo kết nối thành công đến topic động "server/send/<device_id>"
      String publish_topic = "server/send/" + device_id;
      client.publish(publish_topic.c_str(), "Kết nối thành công");
      Serial.printf("Đã gửi thông báo đến topic '%s': Kết nối thành công\n", publish_topic.c_str());
    } else {
      Serial.print("Lỗi kết nối, rc=");
      Serial.print(client.state());
      Serial.println(" - Thử lại sau 5 giây");
      delay(5000);
    }
  }
}

// Hàm khởi tạo MQTT
void setupMQTT() {
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(mqttCallback);
}

// Hàm loop MQTT
void loopMQTT() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
}
