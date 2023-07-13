#include <Arduino_MKRIoTCarrier.h> 
MKRIoTCarrier carrier; 

int moistPin = A5; 
int ACPIN = 3; 

// Store the threshold parameters
float minHumidity = 60.0;
float maxTemp = 30.0;
float minTemp = 22.0;
float idealTemp = 26.0;

// Variables to store current conditions
float humidity;
float temperature;
int moistValue;

// Air conditioner control
unsigned long AC_start_time = 0;
unsigned long AC_max_duration = 10000;

// Light threshold
int LIGHT_THRESHOLD = 200; 

void setup() {
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
    displayWarning("WARNING: Excessive energy use!");
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
  int light = analogRead(A1);

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

void displayWarning(const char* warning) {
  carrier.display.fillScreen(ST77XX_WHITE);
  carrier.display.setTextColor(ST77XX_RED);
  carrier.display.setTextSize(2);
  carrier.display.setCursor(20, 130);
  carrier.display.println(warning);
  delay(2000);

}

void onUpdateDisplayChange(bool redTide) {
  carrier.display.fillScreen(ST77XX_WHITE);
  carrier.display.setTextColor(ST77XX_RED);
  carrier.display.setTextSize(2);
 
  carrier.display.setCursor(20, 30);
  carrier.display.print("Temp: ");
  char buffer[8];
  snprintf(buffer, sizeof(buffer), "%.4f", temperature);  // print temperature to 4 decimal places
  carrier.display.print(buffer);
  carrier.display.print(" C");

  carrier.display.setCursor(20, 50);
  carrier.display.print("Humi: ");
  snprintf(buffer, sizeof(buffer), "%.4f", humidity);     // print humidity to 4 decimal places
  carrier.display.print(buffer);
  carrier.display.print(" %");
  
  carrier.display.setTextColor(ST77XX_BLUE);
  carrier.display.setCursor(20, 90);
  carrier.display.print("Moist: ");
  carrier.display.print(moistValue);
  carrier.display.print(" %");

  carrier.display.setCursor(20, 110);
  carrier.display.print("Red Tide: ");
  carrier.display.print(redTide ? "Yes" : "No");
  delay(2000);


  if (redTide) {
    displayWarning("WARNING: Red tide!");
    delay(2000);

  }
}

