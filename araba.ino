#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <Servo.h>

#define IN1 16
#define IN2 5
#define IN3 4
#define IN4 0

// Wi-Fi bilgileri
const char* ssid = "xxxx";  // Wi-Fi adı
const char* password = "xxxxx";  // Wi-Fi şifresi

ESP8266WebServer server(80);  // Web sunucusu için port numarası (80)

Servo myservo;
int servoPin = D4;  // Servo motor pin tanımlaması


// Motorları durdurma
void stopMotors() {
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, LOW);
}

// Ön ve arka tekerleri ileri sürmek
void forward() {
  digitalWrite(IN1, HIGH);  // Ön tekerler ileri
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, HIGH);  // Arka tekerler ileri
  digitalWrite(IN4, LOW);
}

// Ön ve arka tekerleri geri sürmek
void backward() {
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, HIGH);  // Ön tekerler geri
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, HIGH);  // Arka tekerler geri
}

// Sağ dönüş (servo ile kontrol)
void turnRight() {
  myservo.write(45);  // Sağ 45 derece
}

// Sol dönüş (servo ile kontrol)
void turnLeft() {
  myservo.write(135);  // Sol 135 derece
}

// Düz sürme (servo ile kontrol)
void straight() {
  myservo.write(90);  // Düz pozisyon
}

// HTML sayfası
String htmlPage = "<html>\
                  <head>\
                    <title>Araba Kontrol</title>\
                    <style>\
                      body { text-align: center; background-color: #f0f0f0; font-family: Arial, sans-serif; }\
                      h1 { color: #333; }\
                      #joystick { width: 200px; height: 200px; background-color: lightgrey; border-radius: 50%; position: relative; margin: 0 auto; }\
                      #stick { width: 50px; height: 50px; background-color: grey; border-radius: 50%; position: absolute; top: 75px; left: 75px; }\
                    </style>\
                  </head>\
                  <body>\
                    <h1>Araba Kontrol</h1>\
                    <div id='joystick'>\
                      <div id='stick'></div>\
                    </div>\
                    <script>\
                      const joystick = document.getElementById('joystick');\
                      const stick = document.getElementById('stick');\
                      let joystickActive = false;\
                      let centerX = joystick.offsetWidth / 2;\
                      let centerY = joystick.offsetHeight / 2;\
                      let command = '';\
                      joystick.addEventListener('touchstart', function(e) {\
                        joystickActive = true;\
                        handleJoystick(e.touches[0].clientX, e.touches[0].clientY);\
                      });\
                      joystick.addEventListener('touchmove', function(e) {\
                        if (joystickActive) {\
                          handleJoystick(e.touches[0].clientX, e.touches[0].clientY);\
                        }\
                      });\
                      joystick.addEventListener('touchend', function(e) {\
                        joystickActive = false;\
                        resetJoystick();\
                        sendCommand('stop');\
                      });\
                      function handleJoystick(x, y) {\
                        const rect = joystick.getBoundingClientRect();\
                        const offsetX = x - rect.left - centerX;\
                        const offsetY = y - rect.top - centerY;\
                        const distance = Math.sqrt(offsetX * offsetX + offsetY * offsetY);\
                        const maxDistance = centerX - stick.offsetWidth / 2;\
                        if (distance < maxDistance) {\
                          stick.style.left = offsetX + centerX - stick.offsetWidth / 2 + 'px';\
                          stick.style.top = offsetY + centerY - stick.offsetHeight / 2 + 'px';\
                        }\
                        if (offsetY < -50) {\
                          sendCommand('forward');\
                        } else if (offsetY > 50) {\
                          sendCommand('backward');\
                        } else if (offsetX < -50) {\
                          sendCommand('left');\
                        } else if (offsetX > 50) {\
                          sendCommand('right');\
                        }\
                      }\
                      function resetJoystick() {\
                        stick.style.left = centerX - stick.offsetWidth / 2 + 'px';\
                        stick.style.top = centerY - stick.offsetHeight / 2 + 'px';\
                      }\
                      function sendCommand(cmd) {\
                        if (cmd !== command) {\
                          command = cmd;\
                          fetch('/' + cmd).then(response => response.text()).then(data => { console.log(data); });\
                        }\
                      }\
                    </script>\
                  </body>\
                </html>";

// Web sunucusuna gelen istekleri işleme
void handleRoot() {
  server.send(200, "text/html", htmlPage);  // HTML sayfasını gönder
}

void handleForward() {
  forward();
  server.send(200, "text/plain", "İleri gidiyor");
}

void handleBackward() {
  backward();
  server.send(200, "text/plain", "Geri gidiyor");
}

void handleStop() {
  stopMotors();
  server.send(200, "text/plain", "Durdu");
}

void handleLeft() {
  turnLeft();
  server.send(200, "text/plain", "Sola dönüyor");
}

void handleRight() {
  turnRight();
  server.send(200, "text/plain", "Sağa dönüyor");
}

void setup() { Serial.begin(9600);
  // Motor pinlerini çıkış olarak ayarla
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);

  // Servo motoru başlat
  myservo.attach(servoPin);
  myservo.write(90);  // İlk pozisyon düz

  // Wi-Fi'ye bağlan
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("WiFi'ye bağlanılıyor...");
  }
  Serial.println("WiFi'ye bağlandı");
Serial.println(WiFi.localIP());

  // Web sunucusunu başlat
  server.on("/", handleRoot);  // Ana sayfa
  server.on("/forward", handleForward);  // İleri komutu
  server.on("/backward", handleBackward);  // Geri komutu
  server.on("/stop", handleStop);  // Durdurma komutu
  server.on("/left", handleLeft);  // Sol komutu
  server.on("/right", handleRight);  // Sağ komutu
  server.begin();  // Sunucuyu başlat
}

void loop() {
  server.handleClient();  // Gelen istekleri dinle
}
