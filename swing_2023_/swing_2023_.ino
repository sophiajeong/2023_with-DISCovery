#include <Arduino_MKRIoTCarrier.h> // Include Arduino MKRIoTCarrier library
MKRIoTCarrier carrier; // Declare MKRIoTCarrier object

int moistPin = A5; // Humidity sensor pin number.
int ACPIN = 3; // This is the digital pin number used to control the air conditioner.

// Store the threshold parameters
float minHumidity = 60.0;
float maxTemp = 30.0;
float minTemp = 22.0;
float idealTemp = 26.0;

// Variables to store current conditions
float humidity;
float temperature;
int moistValue;
bool updateDisplay;

// Air conditioner control
unsigned long AC_start_time = 0;
unsigned long AC_max_duration = 10000; // 10 seconds, adjust this value according to your requirement.

// Light threshold
int LIGHT_THRESHOLD = 200; // Replace with your light threshold

void setup() {
  // Initialize serial communication and wait for the port to open.
  Serial.begin(9600);
  while (!Serial);
  
  carrier.begin();
  pinMode(ACPIN, OUTPUT);

  Serial.println("temperature,humidity,moistValue");
}

void loop() {
  // Check if the air conditioner has been on for too long.
  if (digitalRead(ACPIN) == HIGH && (millis() - AC_start_time) > AC_max_duration) {
    digitalWrite(ACPIN, LOW); 
    carrier.display.fillScreen(ST77XX_WHITE);
    carrier.display.setTextColor(ST77XX_RED);
    carrier.display.setTextSize(2);
    carrier.display.setCursor(20, 130);
    carrier.display.println("WARNING: Excessive energy use!");
  }

  // Read the temperature and humidity
  temperature = carrier.Env.readTemperature();
  humidity = carrier.Env.readHumidity();

  // Read and map moisture value
  int rawMoistValue = analogRead(moistPin);
  moistValue = map(rawMoistValue, 0, 1023, 100, 0);

  // Check for red tide
  bool redTide = redTideOccurs();

  if (redTide && (temperature < minTemp || temperature > maxTemp)) {
    setTemperature(idealTemp);
  }

  // Light level reading
  int light = analogRead(A1); // Replace A1 with your light sensor pin

  // Air conditioner temperature control logic.
  if (light > LIGHT_THRESHOLD) {
    if (temperature > 28.9 && digitalRead(ACPIN) == LOW) {
      digitalWrite(ACPIN, HIGH);
      AC_start_time = millis();
    } else if (temperature < 23.2 && digitalRead(ACPIN) == HIGH) {
      digitalWrite(ACPIN, LOW);
    }
  } else if (digitalRead(ACPIN) == HIGH) {
    digitalWrite(ACPIN, LOW);
  }

  // Output current conditions to the serial monitor.
  Serial.print(temperature, 4);
  Serial.print(",");
  Serial.print(humidity, 4);
  Serial.print(",");
  Serial.println(moistValue);

  onUpdateDisplayChange(redTide);
}

bool redTideOccurs() {
  return humidity >= 20 && temperature >= 25.0;
}

void setTemperature(int desiredTemp) {
  // Here we assume that turning on the relay will set the air conditioner to the desired temperature.
  if (desiredTemp == idealTemp) {
    digitalWrite(ACPIN, HIGH);
    AC_start_time = millis();
  } else {
    digitalWrite(ACPIN, LOW);
  }
}

void onUpdateDisplayChange(bool redTide) {
  // Your original function code goes here
}
