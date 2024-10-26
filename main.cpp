#include <WiFi.h>          // Thư viện để ESP32 kết nối với Wi-Fi
#include <BH1750.h>        // Thư viện để giao tiếp với cảm biến ánh sáng BH1750
#include <Wire.h>          // Thư viện hỗ trợ giao tiếp I2C

// Khai báo tên mạng Wi-Fi và mật khẩu
const char* ssid = "Thanh Long";         // Tên Wi-Fi
const char* password = "0989292741"; // Mật khẩu Wi-Fi

BH1750 lightMeter;         // Tạo một đối tượng BH1750 để làm việc với cảm biến ánh sáng
WiFiServer server(80);     // Tạo một server chạy trên cổng 80 (HTTP)

void setup_wifi() {
  // Kết nối ESP32 với Wi-Fi
  WiFi.begin(ssid, password);      // Bắt đầu quá trình kết nối với Wi-Fi
  while (WiFi.status() != WL_CONNECTED) {   // Chờ kết nối hoàn tất
    delay(1000);                            // Đợi 1 giây
    Serial.println("Connecting to WiFi..."); // In ra thông báo khi đang kết nối
  }
  Serial.println("Connected to WiFi");       // In ra khi kết nối thành công
  Serial.println("Địa chỉ IP là:");
  Serial.println(WiFi.localIP());
}

String webPage() {
  // Hàm trả về HTML cho trang web chính
  String html = "<html><head><title>ESP32 Light Sensor</title></head><body>";
  html += "<h1>Light Intensity: <span id='luxValue'>Loading...</span> lx</h1>";
  
  // JavaScript để gọi AJAX mỗi 2 giây nhằm cập nhật giá trị cường độ ánh sáng
  html += "<script>setInterval(function() {";
  html += "fetch('/lux').then(response => response.text()).then(data => {";
  html += "document.getElementById('luxValue').innerText = data;";
  html += "});";
  html += "}, 2000);</script>";  // AJAX gọi mỗi 2 giây
  html += "</body></html>";
  
  return html;  // Trả về chuỗi HTML
}

void setup() {
  Serial.begin(115200);        // Khởi tạo Serial Monitor với tốc độ baud 115200
  setup_wifi();                 // Gọi hàm setup_wifi() để kết nối Wi-Fi
  server.begin();               // Bắt đầu server HTTP
  
  Wire.begin();                 // Khởi tạo giao tiếp I2C
  if (!lightMeter.begin(BH1750::CONTINUOUS_HIGH_RES_MODE)) { // Khởi tạo cảm biến ánh sáng BH1750
    Serial.println("Error initializing BH1750"); // Thông báo lỗi nếu không khởi tạo được cảm biến
  } else {
    Serial.println("BH1750 initialized");        // Thông báo cảm biến đã được khởi tạo thành công
  }
}

void loop() {
  WiFiClient client = server.available();   // Kiểm tra xem có client nào kết nối đến server không
  if (client) {                             // Nếu có client kết nối
    String request = client.readStringUntil('\r');  // Đọc yêu cầu HTTP của client cho đến ký tự '\r'
    client.flush();                         // Xóa bộ đệm

    // Kiểm tra nếu yêu cầu từ client là đường dẫn "/lux"
    if (request.indexOf("GET /lux") != -1) {
      // Đọc giá trị ánh sáng từ cảm biến
      float lux = lightMeter.readLightLevel();
      
      // Trả về giá trị ánh sáng dưới dạng văn bản
      client.print("HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\n\r\n");
      client.print(lux);  // Trả về giá trị ánh sáng cho client
    } else {
      // Trả về trang HTML chính nếu yêu cầu không phải "/lux"
      client.print("HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n");
      client.print(webPage());  // Gọi hàm webPage() để trả về chuỗi HTML
    }
    client.stop();  // Đóng kết nối với client sau khi xử lý xong yêu cầu
  }
}
