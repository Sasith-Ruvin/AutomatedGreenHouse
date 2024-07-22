#define SOIL_MOISTURE_PIN A0
#define PUMP_PIN 13

// Define threshold for turning on the pump (adjust as needed)
#define MOISTURE_THRESHOLD 10  // adjust according to your requirements

void setup() {
  Serial.begin(9600);
  pinMode(SOIL_MOISTURE_PIN, INPUT);
  pinMode(PUMP_PIN, OUTPUT);
}

void loop() {
  // Read soil moisture
  int soilMoisture = analogRead(SOIL_MOISTURE_PIN);
  // Map the analog reading (0-1023) to a percentage (0-100)
  float moisturePercentage = map(soilMoisture, 0, 1023, 0, 100);
  moisturePercentage = 100 - moisturePercentage;

  // Check if the soil moisture reading is valid
  if (soilMoisture < 0) {
    Serial.println("Failed to read from Soil Moisture sensor!");
    return;
  }

  // Control the pump
  if (moisturePercentage < MOISTURE_THRESHOLD) {
    digitalWrite(PUMP_PIN, HIGH);  // Turn on the pump
  } else {
    digitalWrite(PUMP_PIN, LOW);   // Turn off the pump
  }

  // Send data to ESP32
  Serial.print("SoilMoisture:");
  Serial.print(moisturePercentage);
  Serial.print(",PumpStatus:");
  Serial.println(digitalRead(PUMP_PIN) == HIGH ? "On" : "Off");

  delay(2000);
}
