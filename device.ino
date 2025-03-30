#include <HTTPClient.h>

String get_device_id() {
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


void send_device_id(String device_id) {
  HTTPClient http;
  String url = "https://node_js.hust.media/main_2/external/devices/device_id_insert?device_id=" + device_id + "&device_type=micro_chip&device_model=esp32_s3";
  
  http.begin(url); // Khởi tạo yêu cầu đến URL
  int httpCode = http.GET(); // Thực hiện GET request
  
  if (httpCode > 0) { // Kiểm tra phản hồi
    String payload = http.getString();
    Serial.println("Phản hồi từ server: " + payload);
  } else {
    Serial.println("Lỗi khi gửi yêu cầu GET, mã lỗi: " + String(httpCode));
  }
  
  http.end(); // Đóng kết nối
}