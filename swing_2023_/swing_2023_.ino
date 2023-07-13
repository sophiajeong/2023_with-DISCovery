#include <Arduino_MKRIoTCarrier.h> 
MKRIoTCarrier carrier; // MKRIoTCarrier 객체를 생성합니다.

// 센서 및 장치에 사용되는 핀을 설정합니다.
int moistPin = A5; // 토양 습도 센서 핀
int ACPIN = 3; // 에어컨 제어 핀

// 임계값을 설정합니다.
float minHumidity = 60.0;
float maxTemp = 30.0;
float minTemp = 22.0;
float idealTemp = 26.0;

// 현재 조건을 저장하는 변수를 생성합니다.
float humidity;
float temperature;
int moistValue;

// 에어컨 작동 시간을 제어하는 변수를 설정합니다.
unsigned long AC_start_time = 0;
unsigned long AC_max_duration = 10000; // 에어컨 최대 작동 시간

// 빛 임계값을 설정합니다.
int LIGHT_THRESHOLD = 200; 

// 최근 업데이트 시간을 저장하는 변수를 추가합니다.
unsigned long lastUpdateTime = 0;

void setup() {
  Serial.begin(9600); // 시리얼 통신을 시작합니다.
  while (!Serial); // 시리얼 포트가 준비될 때까지 기다립니다.
  
  carrier.begin(); // MKRIoTCarrier를 초기화합니다.
  pinMode(ACPIN, OUTPUT); // 에어컨 제어 핀을 출력으로 설정합니다.

  // 시리얼 모니터에 메시지를 출력합니다.
  Serial.println("temperature,humidity,moistValue"); 
}

void loop() {
  // 에어컨이 너무 오래 작동하고 있는지 확인합니다.
  if (digitalRead(ACPIN) == HIGH && (millis() - AC_start_time) > AC_max_duration) {
    digitalWrite(ACPIN, LOW);  // 에어컨을 끕니다.
    displayWarning("WARNING: Excessive energy use!"); // 경고 메시지를 표시합니다.
  }

  // 온도와 습도를 읽습니다.
  temperature = carrier.Env.readTemperature(); 
  humidity = carrier.Env.readHumidity();

  // 토양 습도 값을 읽고 매핑합니다.
  int rawMoistValue = analogRead(moistPin);
  moistValue = map(rawMoistValue, 0, 1023, 100, 0);

  // 붉은 조류 발생 여부를 확인합니다.
  bool redTide = redTideOccurs();

  // 붉은 조류가 발생하고 온도가 너무 높거나 낮으면 에어컨을 켜서 온도를 조절합니다.
  if (redTide && (temperature < minTemp || temperature > maxTemp)) {
    setTemperature(idealTemp);
  }

  // 빛 센서를 읽습니다.
  int light = analogRead(A1);

  // 빛이 임계값보다 높으면 에어컨을 제어합니다.
  if (light > LIGHT_THRESHOLD) {
    if (temperature > 28.9 && digitalRead(ACPIN) == LOW) {
      digitalWrite(ACPIN, HIGH); // 에어컨을 켭니다.
      AC_start_time = millis(); // 에어컨 작동 시작 시간을 기록합니다.
    } else if (temperature < 23.2 && digitalRead(ACPIN) == HIGH) {
      digitalWrite(ACPIN, LOW); // 에어컨을 끕니다.
    }
  } else if (digitalRead(ACPIN) == HIGH) {
    digitalWrite(ACPIN, LOW); // 에어컨을 끕니다.
  }

  // 현재 조건을 시리얼 모니터에 출력합니다.
  Serial.print(temperature, 4);
  Serial.print(",");
  Serial.print(humidity, 4);
  Serial.print(",");
  Serial.println(moistValue);

  // 디스플레이를 2초마다 업데이트합니다.
  if (millis() - lastUpdateTime > 2000) {
    onUpdateDisplayChange(redTide);
    lastUpdateTime = millis();
  }
}

// 붉은 조류 발생 여부를 반환하는 함수입니다.
bool redTideOccurs() {
  return moistValue >= 20 && temperature >= 25.0;
}

// 원하는 온도로 에어컨을 설정하는 함수입니다.
void setTemperature(int desiredTemp) {
  if (desiredTemp == idealTemp) {
    digitalWrite(ACPIN, HIGH); // 에어컨을 켭니다.
    AC_start_time = millis(); // 에어컨 작동 시작 시간을 기록합니다.
  } else {
    digitalWrite(ACPIN, LOW); // 에어컨을 끕니다.
  }
}

// 경고 메시지를 디스플레이에 표시하는 함수입니다.
void displayWarning(const char* warning) {
  carrier.display.fillScreen(ST77XX_WHITE);
  carrier.display.setTextColor(ST77XX_RED);
  carrier.display.setTextSize(2);
  carrier.display.setCursor(20, 130);
  carrier.display.println(warning);
  delay(2000); // 2초 동안 딜레이합니다.
}

// 디스플레이에 정보를 업데이트하는 함수입니다.
void onUpdateDisplayChange(bool redTide) {
  carrier.display.fillScreen(ST77XX_WHITE);
  carrier.display.setTextColor(ST77XX_RED);
  carrier.display.setTextSize(2);

  // 온도 정보를 디스플레이에 표시합니다.
  carrier.display.setCursor(20, 30);
  carrier.display.print("Temp: ");
  carrier.display.print(temperature);
  carrier.display.print(" C");

  // 습도 정보를 디스플레이에 표시합니다.
  carrier.display.setCursor(20, 50);
  carrier.display.print("Humi: ");
  carrier.display.print(humidity);
  carrier.display.print(" %");
  
  // 토양 습도 정보를 디스플레이에 표시합니다.
  carrier.display.setTextColor(ST77XX_BLUE);
  carrier.display.setCursor(20, 90);
  carrier.display.print("Moist: ");
  carrier.display.print(moistValue);
  carrier.display.print(" %");

  // 붉은 조류 발생 정보를 디스플레이에 표시합니다.
  carrier.display.setCursor(20, 110);
  carrier.display.print("Red Tide: ");
  carrier.display.print(redTide ? "Yes" : "No");
  delay(2000);
  
  if (redTide) {
    displayWarning("WARNING: Red tide!");
    delay(2000);
  }
}