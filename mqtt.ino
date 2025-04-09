#include <PubSubClient.h>
#include <ArduinoJson.h>
#include "process.h" // Giả sử process.h chứa các định nghĩa cần thiết

// Thông tin MQTT broker
const char *mqtt_server = "vip.tecom.pro";
const int mqtt_port = 1883;

// Khởi tạo client MQTT
WiFiClient espClient;
PubSubClient client(espClient);

// Hàm xử lý JSON và reset server nếu nhận lệnh reset
void processJsonAndReset(const String &jsonMessage)
{
  DynamicJsonDocument doc(256);
  DeserializationError error = deserializeJson(doc, jsonMessage);
  if (error)
  {
    Serial.println("Lỗi khi phân tích cú pháp JSON trong processJsonAndReset.");
    return;
  }

  // Kiểm tra nếu có key "command_code" và giá trị bằng 1
  if (doc.containsKey("command_code") && doc["command_code"] == 1)
  {
    Serial.println("reset server...");
    // Thực hiện các xử lý khác nếu cần trước khi reset
    ESP.restart();
  }
  else if (doc.containsKey("command_code") && doc["command_code"] == 2)
  {
    Serial.println("reset mạng...");
    // Thực hiện các xử lý khác nếu cần trước khi reset
  }
  else if (doc.containsKey("command_code") && doc["command_code"] == 3)
  {
    Serial.println("reset audio...");
    // Thực hiện các xử lý khác nếu cần trước khi reset
    radio_restart(audio);
  }
  else if (doc.containsKey("command_code") && doc["command_code"] == 4)
  {
    const int command_action = doc["command_action"]; // Ép kiểu rõ ràng nếu là int
    Serial.println("audio control: " + String(command_action));
    // Gọi hàm setVolume (giả sử hàm này nhận int)
    volume = command_action; // Dễ dàng cập nhật giá trị String
    audio.setVolume(command_action);
    setVolume(volume);
  }
}

// Hàm callback xử lý tin nhắn MQTT
void mqttCallback(char *topic, unsigned char *payload, unsigned int length)
{
  String message;
  for (unsigned int i = 0; i < length; i++)
  {
    message += (char)payload[i];
  }
  Serial.printf("Nhận được tin nhắn từ topic '%s': %s\n", topic, message.c_str());

  // Nếu tin nhắn dạng JSON, gọi hàm xử lý riêng
  if (message.startsWith("{"))
  {
    processJsonAndReset(message);
  }

  // Xây dựng topic gửi phản hồi động: "device/send/<device_id>"
  String publish_topic = "device/send/" + device_id;
  String response = "Đã nhận: " + message;
  client.publish(publish_topic.c_str(), response.c_str());
  Serial.printf("Đã gửi phản hồi đến topic '%s': %s\n", publish_topic.c_str(), response.c_str());
}

// Hàm kết nối lại MQTT
void reconnect()
{
  while (!client.connected())
  {
    Serial.print("Kết nối MQTT...");
    String clientId = "ESP32Client-";
    clientId += String(random(0xffff), HEX);
    if (client.connect(clientId.c_str()))
    {
      Serial.println("Kết nối thành công!");

      // Xây dựng topic subscribe động: "server/send/<device_id>"
      String subscribe_topic = "server/send/" + device_id;
      client.subscribe(subscribe_topic.c_str());

      // Gửi thông báo kết nối thành công đến topic động "server/send/<device_id>"
      String publish_topic = "server/send/" + device_id;
      client.publish(publish_topic.c_str(), "Kết nối thành công");
      Serial.printf("Đã gửi thông báo đến topic '%s': Kết nối thành công\n", publish_topic.c_str());
    }
    else
    {
      Serial.print("Lỗi kết nối, rc=");
      Serial.print(client.state());
      Serial.println(" - Thử lại sau 5 giây");
      delay(5000);
    }
  }
}

// Hàm khởi tạo MQTT
void setupMQTT()
{
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(mqttCallback);
}

// Hàm loop MQTT
void loopMQTT()
{
  if (!client.connected())
  {
    reconnect();
  }
  client.loop();
}
