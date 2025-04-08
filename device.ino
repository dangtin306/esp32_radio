#include <HTTPClient.h>

String get_device_id()
{
  // Serial.begin(115200);
  delay(300);
  // Lấy giá trị MAC từ efuse của ESP32
  uint64_t mac = ESP.getEfuseMac();

  // Tách 32-bit thấp
  uint32_t low = mac & 0xFFFFFFFF;
  // Tách 32-bit cao và tính modulo 0xFFFFFFFF (có thể dùng toán tử & thay cho % cũng được)
  uint32_t high = (mac >> 32) % 0xFFFFFFFF;

  // Kết hợp low và high thành 64-bit.
  // Lưu ý: Sử dụng bit shifting để kết hợp 2 số 32-bit thành 1 số 64-bit
  uint64_t fullMAC = ((uint64_t)high << 32) | low;

  // In ra Serial các giá trị chi tiết
  Serial.printf("Low: %u\n", low);
  Serial.printf("High: %u\n", high);
  Serial.printf("Full: %llu\n", fullMAC);

  // Trả về fullMAC dưới dạng chuỗi hex
  return String(fullMAC, HEX);
}

void send_device_id(String device_id)
{
  HTTPClient http;
  String url = "https://node_js.hust.media/main_2/external/devices/device_id_insert?device_id=" + device_id + "&device_type=micro_chip&device_model=esp32_s3";

  http.begin(url);           // Khởi tạo yêu cầu đến URL
  int httpCode = http.GET(); // Thực hiện GET request

  if (httpCode > 0)
  { // Kiểm tra phản hồi
    String payload = http.getString();
    Serial.println("Phản hồi từ server: " + payload);
  }
  else
  {
    Serial.println("Lỗi khi gửi yêu cầu GET, mã lỗi: " + String(httpCode));
  }

  http.end(); // Đóng kết nối
}

void get_link_live(String device_id) {
  HTTPClient http;
  String req_url = "https://node_js.hust.media/main_2/audio/live/get_link_live?device_id=" + device_id;
  http.begin(req_url);
  int httpCode = http.GET();

  if (httpCode > 0) {
    String payload = http.getString();
    Serial.println("Phản hồi từ server: " + payload);

    const size_t capacity = JSON_OBJECT_SIZE(2) + JSON_OBJECT_SIZE(3);
    DynamicJsonDocument doc(capacity);

    DeserializationError error = deserializeJson(doc, payload);
    if (error) {
      Serial.print("Lỗi parse JSON: ");
      Serial.println(error.f_str());
      return;
    }

    // Cập nhật biến toàn cục live_url_audio với giá trị mới từ JSON
    const char* new_url = doc["api_results"];
    if (new_url) {
      live_url_audio = new_url; // Dễ dàng cập nhật giá trị String
      Serial.println("Live link cập nhật: " + live_url_audio);
    } else {
      Serial.println("Không tìm thấy 'api_results' trong JSON.");
    }
  } else {
    Serial.println("Lỗi khi gửi yêu cầu GET, mã lỗi: " + String(httpCode));
  }

  http.end();
}

void radio_restart(Audio &audio) {
  delay(200);
  audio.setVolume(15);
  audio.setPinout(3, 1, 9);
  audio.connecttohost(live_url_audio.c_str());
}