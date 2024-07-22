// Import required libraries
#include "WiFi.h"
#include "ESPAsyncWebServer.h"
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <Arduino.h>

// Replace with your network credentials
const char* ssid = "The Republic";
const char* password = "lotr1234";

#define RXp2 16
#define TXp2 17
#define DHTPIN 27

#define DHTTYPE    DHT22

float soilMoisture = 0.0;
String pumpStatus = "";

DHT dht(DHTPIN, DHTTYPE);

// Create AsyncWebServer object on port 80
AsyncWebServer server(80);

String readDHTTemperature() {
  // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
  // Read temperature as Celsius (the default)
  float t = dht.readTemperature();
  // Read temperature as Fahrenheit (isFahrenheit = true)
  //float t = dht.readTemperature(true);
  // Check if any reads failed and exit early (to try again).
  if (isnan(t)) {    
    Serial.println("Failed to read from DHT sensor!");
    return "--";
  }
  else {
    Serial.println(t);
    return String(t);
  }
}

String readDHTHumidity() {
  // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
  float h = dht.readHumidity();
  if (isnan(h)) {
    Serial.println("Failed to read from DHT sensor!");
    return "--";
  }
  else {
    Serial.println(h);
    return String(h);
  }
}

float readSoilMoisture() {
  return soilMoisture;
}

String isPumpOn() {
  return String(pumpStatus);
}

const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML><html>
<head>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <link rel="stylesheet" href="https://use.fontawesome.com/releases/v5.7.2/css/all.css" integrity="sha384-fnmOCqbTlWIlj8LyTjo7mOUStjsKC4pOpQbqyi7RrhN7udi9RwhKkMHpvLbHG9Sr" crossorigin="anonymous">
  <style>
    html {
     font-family: Arial;
     display: inline-block;
     margin: 0px auto;
     text-align: center;
    }
    h2 { font-size: 3.0rem; }
    p { font-size: 3.0rem; }
    .units { font-size: 1.2rem; }
    .dht-labels{
      font-size: 1.5rem;
      vertical-align:middle;
      padding-bottom: 15px;
    }
  </style>
</head>
<body>
  <h2>ESP32 DHT Server</h2>
  <p>
    <i class="fas fa-thermometer-half" style="color:#059e8a;"></i> 
    <span class="dht-labels">Temperature</span> 
    <span id="temperature">%TEMPERATURE%</span>
    <sup class="units">&deg;C</sup>
  </p>
  <p>
    <i class="fas fa-tint" style="color:#00add6;"></i> 
    <span class="dht-labels">Humidity</span>
    <span id="humidity">%HUMIDITY%</span>
    <sup class="units">&percnt;</sup>
  </p>
  <p>
    <i class="fas fa-tint" style="color:#faad14;"></i> 
    <span class="dht-labels">Soil Moisture</span>
    <span id="soil_moisture">%SOIL_MOISTURE%</span>
    <sup class="units">&percnt;</sup>
  </p>
  <p>
    <i class="fas fa-power-off" style="color:#ff0000;"></i> 
    <span class="dht-labels">Pump Status</span>
    <span id="pump_status">%PUMP_STATUS%</span>
  </p>
</body>
<script>
setInterval(function ( ) {
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      document.getElementById("temperature").innerHTML = this.responseText;
    }
  };
  xhttp.open("GET", "/temperature", true);
  xhttp.send();
}, 10000 ) ;

setInterval(function ( ) {
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      document.getElementById("humidity").innerHTML = this.responseText;
    }
  };
  xhttp.open("GET", "/humidity", true);
  xhttp.send();
}, 10000 ) ;
setInterval(function () {
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      document.getElementById("soil_moisture").innerHTML = this.responseText;
    }
  };
  xhttp.open("GET", "/soilmoisture", true);
  xhttp.send();
}, 10000);
setInterval(function () {
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      document.getElementById("pump_status").innerHTML = this.responseText;
    }
  };
  xhttp.open("GET", "/pumpstatus", true);
  xhttp.send();
}, 10000);
</script>
</html>)rawliteral";

// Replaces placeholder with sensor values
String processor(const String& var){
  //Serial.println(var);
  if(var == "TEMPERATURE"){
    return readDHTTemperature();
  }
  else if(var == "HUMIDITY"){
    return readDHTHumidity();
  }
  else if(var == "SOIL_MOISTURE"){
    return String(readSoilMoisture());
  }
  else if(var == "PUMP_STATUS"){
    return isPumpOn();
  }
  return String();
}

void setup(){
  // Serial port for debugging purposes
  Serial.begin(115200);
  Serial2.begin(9600, SERIAL_8N1, RXp2, TXp2);

  dht.begin();

  // Connect to Wi-Fi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi..");
  }

  // Print ESP32 Local IP Address
  Serial.println(WiFi.localIP());

  // Route for root / web page
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/html", index_html, processor);
  });
  server.on("/temperature", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/plain", readDHTTemperature().c_str());
  });
  server.on("/humidity", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/plain", readDHTHumidity().c_str());
  });
  server.on("/soilmoisture", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/plain", String(readSoilMoisture()).c_str());
  });
  server.on("/pumpstatus", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/plain", isPumpOn().c_str());;
  });

  // Start server
  server.begin();
}
 
void loop(){
  if (Serial2.available()) {
    String message = Serial2.readStringUntil('\n');
    Serial.println("Message Received: " + message);
    
    // Parse the message and extract values
    int soilMoistureIndex = message.indexOf(",SoilMoisture:") + 14;
    int pumpStatusIndex = message.indexOf(",PumpStatus:") + 12;

    soilMoisture = message.substring(soilMoistureIndex, message.indexOf(",", soilMoistureIndex)).toFloat();
    pumpStatus = message.substring(pumpStatusIndex);

    // Print the extracted values
     Serial.println(WiFi.localIP());
    
    Serial.print("Soil Moisture: ");
    Serial.println(soilMoisture);
    Serial.print("Pump Status: ");
    Serial.println(pumpStatus);
  }

  // Add a delay to prevent rapid readings and pump switching
  delay(1000); // Adjust delay as needed
}
