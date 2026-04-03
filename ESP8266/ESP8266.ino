#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <WiFiUdp.h>
#include <Servo.h>

// Router WiFi settings (STA mode)
const char* WIFI_SSID = "SYSU-HANLU";
const char* WIFI_PASS = "hll123456";

// Servo settings (GPIO14 = D5 on many NodeMCU boards)
const uint8_t SERVO_PIN = 5;
const int ANGLE_MIN = 0;
const int ANGLE_MAX = 180;

ESP8266WebServer server(80);
WiFiUDP udp;
Servo myServo;
int currentAngle = 90;
const uint16_t UDP_PORT = 4210;

const char INDEX_HTML[] PROGMEM = R"rawliteral(
<!doctype html>
<html>
<head>
  <meta charset="utf-8" />
  <meta name="viewport" content="width=device-width,initial-scale=1" />
  <title>ESP8266 Servo Control</title>
  <style>
    body { font-family: Arial, sans-serif; margin: 24px; background: #f5f7fb; color: #222; }
    .box { max-width: 460px; margin: 0 auto; padding: 20px; background: #fff; border-radius: 12px; box-shadow: 0 6px 20px rgba(0,0,0,0.08); }
    h1 { font-size: 22px; margin: 0 0 16px; }
    .row { margin: 16px 0; }
    input[type=range] { width: 100%; }
    .angle { font-size: 28px; font-weight: bold; text-align: center; margin-top: 8px; }
    .hint { font-size: 13px; color: #666; }
    button { width: 100%; border: 0; border-radius: 8px; padding: 10px; font-size: 16px; cursor: pointer; background: #0b63f6; color: #fff; }
  </style>
</head>
<body>
  <div class="box">
    <h1>ESP8266 舵机控制</h1>
    <div class="row">
      <input id="slider" type="range" min="0" max="180" value="90" />
      <div id="angle" class="angle">90°</div>
    </div>
    <div class="row">
      <button id="sendBtn">发送角度</button>
    </div>
    <div class="hint">开发板将连接到路由器 WiFi，请在串口监视器查看 IP，然后在同一局域网访问该 IP</div>
  </div>

  <script>
    const slider = document.getElementById('slider');
    const angleText = document.getElementById('angle');
    const sendBtn = document.getElementById('sendBtn');

    function sendAngle(v) {
      fetch('/set?angle=' + v)
        .then(r => r.text())
        .then(t => console.log(t))
        .catch(e => console.log(e));
    }

    slider.addEventListener('input', () => {
      angleText.textContent = slider.value + '°';
    });

    slider.addEventListener('change', () => {
      sendAngle(slider.value);
    });

    sendBtn.addEventListener('click', () => {
      sendAngle(slider.value);
    });
  </script>
</body>
</html>
)rawliteral";

void handleRoot() {
  server.send_P(200, "text/html", INDEX_HTML);
}

void handleSetAngle() {
  if (!server.hasArg("angle")) {
    server.send(400, "text/plain", "missing angle");
    return;
  }

  int angle = server.arg("angle").toInt();
  angle = constrain(angle, ANGLE_MIN, ANGLE_MAX);
  currentAngle = angle;
  myServo.write(currentAngle);

  String msg = "angle=" + String(currentAngle);
  server.send(200, "text/plain", msg);
}

void handleStatus() {
  String json = "{\"angle\":" + String(currentAngle) + "}";
  server.send(200, "application/json", json);
}

void setup() {
  Serial.begin(115200);
  delay(100);
  Serial.println();
  myServo.attach(SERVO_PIN);
  myServo.write(currentAngle);

  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASS);

  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println();

  server.on("/", HTTP_GET, handleRoot);
  server.on("/set", HTTP_GET, handleSetAngle);
  server.on("/status", HTTP_GET, handleStatus);
  server.begin();
  udp.begin(UDP_PORT);

  Serial.println();
  Serial.println("WiFi connected");
  Serial.print("SSID: ");
  Serial.println(WIFI_SSID);
  Serial.print("IP: ");
  Serial.println(WiFi.localIP());
  Serial.print("UDP port: ");
  Serial.println(UDP_PORT);
}

void loop() {
  server.handleClient();

  int packetSize = udp.parsePacket();
  if (packetSize > 0) {
    char buf[32];
    int len = udp.read(buf, sizeof(buf) - 1);
    if (len > 0) {
      buf[len] = '\0';
      int angle = atoi(buf);
      angle = constrain(angle, ANGLE_MIN, ANGLE_MAX);
      currentAngle = angle;
      myServo.write(currentAngle);
    }
  }
}
