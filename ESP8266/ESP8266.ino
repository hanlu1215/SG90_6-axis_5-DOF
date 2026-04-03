#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <WiFiUdp.h>
#include <Servo.h>

// Router WiFi settings (STA mode)
const char* WIFI_SSID = "SYSU-HANLU";
const char* WIFI_PASS = "hll123456";

const int ANGLE_MIN = 0;
const int ANGLE_MAX = 180;
const uint8_t SERVO_COUNT = 6;
const uint8_t SERVO_PINS[SERVO_COUNT] = {12, 14, 16, 0, 4, 5};

ESP8266WebServer server(80);
WiFiUDP udp;
Servo servos[SERVO_COUNT];
int currentAngles[SERVO_COUNT] = {90, 90, 90, 90, 90, 90};
const uint16_t UDP_PORT = 4210;


int clampAngle(int angle) {
  return constrain(angle, ANGLE_MIN, ANGLE_MAX);
}


bool parseServoIndex() {
  if (!server.hasArg("servo")) {
    return false;
  }
  int index = server.arg("servo").toInt();
  return index >= 1 && index <= SERVO_COUNT;
}


void applyServoAngle(uint8_t servoIndex, int angle) {
  if (servoIndex >= SERVO_COUNT) {
    return;
  }
  currentAngles[servoIndex] = clampAngle(angle);
  servos[servoIndex].write(currentAngles[servoIndex]);
}

const char INDEX_HTML[] PROGMEM = R"rawliteral(
<!doctype html>
<html>
<head>
  <meta charset="utf-8" />
  <meta name="viewport" content="width=device-width,initial-scale=1" />
  <title>ESP8266 Servo Control</title>
  <style>
    :root { color-scheme: light; }
    body { font-family: Arial, sans-serif; margin: 0; padding: 24px; background: linear-gradient(135deg, #eef3ff 0%, #f8fafc 45%, #eef9f4 100%); color: #1f2937; }
    .shell { max-width: 920px; margin: 0 auto; }
    .hero { margin-bottom: 20px; padding: 20px; border-radius: 18px; background: rgba(255,255,255,0.84); box-shadow: 0 10px 30px rgba(15, 23, 42, 0.08); backdrop-filter: blur(10px); }
    h1 { margin: 0 0 8px; font-size: 28px; }
    .hint { font-size: 13px; color: #64748b; line-height: 1.5; }
    .grid { display: grid; grid-template-columns: repeat(auto-fit, minmax(260px, 1fr)); gap: 14px; }
    .card { background: rgba(255,255,255,0.9); border-radius: 16px; padding: 16px; box-shadow: 0 8px 24px rgba(15, 23, 42, 0.08); }
    .title { display: flex; justify-content: space-between; align-items: center; margin-bottom: 12px; font-weight: 700; }
    .angle { font-size: 26px; font-weight: 700; color: #0f172a; }
    input[type=range] { width: 100%; }
    .actions { margin-top: 18px; display: flex; gap: 12px; flex-wrap: wrap; }
    button { border: 0; border-radius: 10px; padding: 10px 14px; font-size: 15px; cursor: pointer; background: #0b63f6; color: #fff; }
    button.secondary { background: #e2e8f0; color: #0f172a; }
  </style>
</head>
<body>
  <div class="shell">
    <div class="hero">
      <h1>ESP8266 6 路舵机控制</h1>
      <div class="hint">舵机编号对应引脚：1=GPIO12, 2=GPIO14, 3=GPIO16, 4=GPIO0, 5=GPIO4, 6=GPIO5。网页和 UDP 都支持独立控制。</div>
      <div class="actions">
        <button id="refreshBtn" class="secondary">刷新状态</button>
      </div>
    </div>
    <div id="grid" class="grid"></div>
  </div>

  <script>
    const servoCount = 6;
    const grid = document.getElementById('grid');
    const refreshBtn = document.getElementById('refreshBtn');
    const cards = [];

    function sendAngle(index, angle) {
      return fetch('/set?servo=' + index + '&angle=' + angle)
        .then(r => r.text());
    }

    function makeCard(index) {
      const card = document.createElement('div');
      card.className = 'card';
      card.innerHTML = `
        <div class="title"><span>舵机 ${index}</span><span id="angle-${index}" class="angle">90°</span></div>
        <input id="slider-${index}" type="range" min="0" max="180" value="90" />
      `;

      const slider = card.querySelector(`#slider-${index}`);
      const angleText = card.querySelector(`#angle-${index}`);

      slider.addEventListener('input', () => {
        angleText.textContent = slider.value + '°';
      });

      slider.addEventListener('change', () => {
        sendAngle(index, slider.value).catch(e => console.log(e));
      });

      cards.push({ index, slider, angleText });
      return card;
    }

    for (let i = 1; i <= servoCount; i++) {
      grid.appendChild(makeCard(i));
    }

    function refreshStatus() {
      fetch('/status')
        .then(r => r.json())
        .then(data => {
          if (!data.angles || data.angles.length < servoCount) return;
          cards.forEach(card => {
            const value = data.angles[card.index - 1];
            card.slider.value = value;
            card.angleText.textContent = value + '°';
          });
        })
        .catch(e => console.log(e));
    }

    refreshBtn.addEventListener('click', refreshStatus);
    refreshStatus();
  </script>
</body>
</html>
)rawliteral";

void handleRoot() {
  server.send_P(200, "text/html", INDEX_HTML);
}

void handleSetAngle() {
  if (!server.hasArg("servo") || !server.hasArg("angle")) {
    server.send(400, "text/plain", "missing servo or angle");
    return;
  }

  int servoIndex = server.arg("servo").toInt();
  int angle = clampAngle(server.arg("angle").toInt());
  if (servoIndex < 1 || servoIndex > SERVO_COUNT) {
    server.send(400, "text/plain", "invalid servo index");
    return;
  }

  applyServoAngle((uint8_t)(servoIndex - 1), angle);

  String msg = "servo=" + String(servoIndex) + ",angle=" + String(currentAngles[servoIndex - 1]);
  server.send(200, "text/plain", msg);
}

void handleStatus() {
  String json = "{\"angles\":[";
  for (uint8_t i = 0; i < SERVO_COUNT; i++) {
    json += String(currentAngles[i]);
    if (i + 1 < SERVO_COUNT) {
      json += ",";
    }
  }
  json += "]}";
  server.send(200, "application/json", json);
}

void handleSetUdpPacket() {
  char buf[48];
  int len = udp.read(buf, sizeof(buf) - 1);
  if (len <= 0) {
    return;
  }
  buf[len] = '\0';

  int servoIndex = 0;
  int angle = 0;
  if (sscanf(buf, "%d,%d", &servoIndex, &angle) == 2) {
    if (servoIndex >= 1 && servoIndex <= SERVO_COUNT) {
      applyServoAngle((uint8_t)(servoIndex - 1), angle);
    }
    return;
  }

  angle = atoi(buf);
  applyServoAngle(0, angle);
}

void setup() {
  Serial.begin(115200);
  delay(100);
  Serial.println();

  for (uint8_t i = 0; i < SERVO_COUNT; i++) {
    servos[i].attach(SERVO_PINS[i]);
    servos[i].write(currentAngles[i]);
  }

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
    handleSetUdpPacket();
  }
}
