/*  
  Rui Santos & Sara Santos - Random Nerd Tutorials
  https://RandomNerdTutorials.com/esp32-wi-fi-car-robot-arduino/
  Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files.
  The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
*/

#include <WiFi.h>
#include <WebServer.h>

// Replace with your network credentials
const char* ssid = "LAN of the free";  //Replace with your network ID
const char* password = "nerD123$";  // Replace with your network password

// Create an instance of the WebServer on port 80
WebServer server(80);

// Motor 1
int motor1Pin1 = 27; 
int motor1Pin2 = 26; 
int enable1Pin = 14;

// Motor 2
int motor2Pin1 = 33; 
int motor2Pin2 = 25; 
int enable2Pin = 32;

// Setting PWM properties
const int freq = 200;
const int resolution = 8;
int dutyCycle = 0;

bool invertLR = false;
bool invertUD = false;

String lastLog = "";

String valueString = String(0);

void handleRoot() {
  const char html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML><html>
<head>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <link rel="icon" href="data:,">
  <style>
    html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center; }
    .button { -webkit-user-select: none; -moz-user-select: none; -ms-user-select: none; user-select: none; background-color: #4CAF50; border: none; color: white; padding: 12px 28px; text-decoration: none; font-size: 26px; margin: 1px; cursor: pointer; }
    .button2 { background-color: #555555; }

    .switch {
      position: relative;
      display: inline-block;
      width: 60px;
      height: 34px;
      margin-left: 10px;
    }

    .switch input {
      opacity: 0;
      width: 0;
      height: 0;
    }

    .slider {
      position: absolute;
      cursor: pointer;
      top: 0; left: 0; right: 0; bottom: 0;
      background-color: #ccc;
      transition: .4s;
      border-radius: 34px;
    }

    .slider:before {
      position: absolute;
      content: "";
      height: 26px; width: 26px;
      left: 4px; bottom: 4px;
      background-color: white;
      transition: .4s;
      border-radius: 50%;
    }

    input:checked + .slider {
      background-color: #2196F3;
    }

    input:checked + .slider:before {
      transform: translateX(26px);
    }
  </style>
  <script>
    function moveUp() { fetch('/up'); }
    function moveLeft() { fetch('/left'); }
    function stopRobot() { fetch('/stop'); }
    function moveRight() { fetch('/right'); }
    function moveDown() { fetch('/down'); }

    function toggleSwitch(direction) {
      fetch('/toggle' + direction);
    }

    function updateMotorSpeed(pos) {
      document.getElementById('motorSpeed').innerHTML = pos;
      fetch(`/speed?value=${pos}`);
    }

    function fetchLog() {
      fetch('/log')
        .then(response => response.text())
        .then(data => {
          document.getElementById('logText').innerText = data;
        });
    }

    setInterval(fetchLog, 1000);
  </script>
</head>
<body>
  <h1>ESP32 Pan/Tilt Control</h1>
  <p><button class="button" onclick="moveUp()">UP</button></p>
  <div style="clear: both;">
    <p>
      <button class="button" onclick="moveLeft()">LEFT</button>
      <button class="button button2" onclick="stopRobot()">STOP</button>
      <button class="button" onclick="moveRight()">RIGHT</button>
    </p>
  </div>
  <p><button class="button" onclick="moveDown()">DOWN</button></p>
  <p>Motor Speed: <span id="motorSpeed">0</span></p>
  <input type="range" min="0" max="100" step="10" id="motorSlider" oninput="updateMotorSpeed(this.value)" value="0"/>

  <p>
    Invert Left/Right:
    <label class="switch">
      <input type="checkbox" id="lrSwitch" onchange="toggleSwitch('LR')" %LRSTATE%>
      <span class="slider"></span>
    </label>
  </p>

  <p>
    Invert Up/Down:
    <label class="switch">
      <input type="checkbox" id="udSwitch" onchange="toggleSwitch('UD')" %UDSTATE%>
      <span class="slider"></span>
    </label>
  </p>

  <p>Last Action: <span id="logText">None</span></p>
</body>
</html>
)rawliteral";
  String page = html;
  page.replace("%LRSTATE%", invertLR ? "checked" : "");
  page.replace("%UDSTATE%", invertUD ? "checked" : "");
  server.send(200, "text/html", html);
}

void handleUp() {
  lastLog = "Up"; 
  Serial.println(lastLog);
  if (invertUD) {
    digitalWrite(motor1Pin1, HIGH);
    digitalWrite(motor1Pin2, LOW);
  } else {
    digitalWrite(motor1Pin1, LOW);
    digitalWrite(motor1Pin2, HIGH); 
  }
  server.send(200);
}

void handleDown() {
  lastLog = "Down"; 
  Serial.println(lastLog);
  if (invertUD) {
    digitalWrite(motor1Pin1, LOW);
    digitalWrite(motor1Pin2, HIGH); 
  } else {
    digitalWrite(motor1Pin1, HIGH);
    digitalWrite(motor1Pin2, LOW);
  }
  server.send(200);
}

void handleLeft() {
  lastLog = "Left"; 
  Serial.println(lastLog);
  if (invertLR) {
    digitalWrite(motor2Pin1, HIGH);
    digitalWrite(motor2Pin2, LOW);
  } else {
    digitalWrite(motor2Pin1, LOW);
    digitalWrite(motor2Pin2, HIGH);
  }
  server.send(200);
}

void handleRight() {
  lastLog = "Right"; 
  Serial.println(lastLog);
  if (invertLR) {
    digitalWrite(motor2Pin1, LOW);
    digitalWrite(motor2Pin2, HIGH);
  } else {
    digitalWrite(motor2Pin1, HIGH);
    digitalWrite(motor2Pin2, LOW);
  }
  server.send(200);
}

void handleStop() {
  lastLog = "Stop"; 
  Serial.println(lastLog);
  digitalWrite(motor1Pin1, LOW); 
  digitalWrite(motor1Pin2, LOW); 
  digitalWrite(motor2Pin1, LOW);
  digitalWrite(motor2Pin2, LOW);   
  server.send(200);
}

void handleSpeed() {
  if (server.hasArg("value")) {
    valueString = server.arg("value");
    int value = valueString.toInt();
    if (value == 0) {
      ledcWrite(enable1Pin, 0);
      ledcWrite(enable2Pin, 0);
      digitalWrite(motor1Pin1, LOW); 
      digitalWrite(motor1Pin2, LOW); 
      digitalWrite(motor2Pin1, LOW);
      digitalWrite(motor2Pin2, LOW);   
    } else { 
      dutyCycle = map(value, 5, 100, 20, 255); //value, strtsldrrng, stpsldrrng, strtpwmrng, stppwmrng
      ledcWrite(enable1Pin, dutyCycle);
      ledcWrite(enable2Pin, dutyCycle);
      lastLog = "Motor speed set to " + String(value); 
      Serial.println(lastLog);
      //Serial.println("Motor speed set to " + String(value));
    }
  }
  server.send(200);
}

void setup() {
  Serial.begin(115200);

  // Set the Motor pins as outputs
  pinMode(motor1Pin1, OUTPUT);
  pinMode(motor1Pin2, OUTPUT);
  pinMode(motor2Pin1, OUTPUT);
  pinMode(motor2Pin2, OUTPUT);

  // Configure PWM Pins
  ledcAttach(enable1Pin, freq, resolution);
  ledcAttach(enable2Pin, freq, resolution);
    
  // Initialize PWM with 0 duty cycle
  ledcWrite(enable1Pin, 0);
  ledcWrite(enable2Pin, 0);
  
  // Connect to Wi-Fi
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected.");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  // Define routes
  server.on("/", handleRoot);
  server.on("/up", handleUp);
  server.on("/left", handleLeft);
  server.on("/stop", handleStop);
  server.on("/right", handleRight);
  server.on("/down", handleDown);
  server.on("/speed", handleSpeed);
  server.on("/log", []() {
  server.send(200, "text/plain", lastLog);
});

  // Toggle Directions
  server.on("/toggleLR", []() {
  invertLR = !invertLR;
  lastLog = "Invert L/R: " + String(invertLR); 
  Serial.println(lastLog);
  //Serial.println("Invert L/R: " + String(invertLR));
  server.send(200);
});

  server.on("/toggleUD", []() {
  invertUD = !invertUD;
  lastLog = "Invert U/D: " + String(invertUD); 
  Serial.println(lastLog);
  //Serial.println("Invert U/D: " + String(invertUD));
  server.send(200);
});
  // Start the server
  server.begin();
}

void loop() {
  server.handleClient();
}
