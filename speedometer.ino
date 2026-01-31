#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// ---------- PINS ----------
#define TRIG_A 5
#define ECHO_A 18
#define TRIG_B 32
#define ECHO_B 34
#define BUZZER_PIN 27

// ---------- SETTINGS ----------
#define SENSOR_DISTANCE_CM 10.0   // distance between sensors
#define TRIGGER_DISTANCE 17       // detection range
#define SPEED_LIMIT_MPH 80.0
#define MPH_PER_CMPS 1.5          // 1cm/sec = 1.5 MPH

LiquidCrystal_I2C lcd(0x27, 16, 2);  // I2C LCD

// ---------- STATE ----------
unsigned long timeA = 0;
unsigned long timeB = 0;
bool detectedA = false;
bool detectedB = false;

// ---------- FUNCTIONS ----------
long readDistanceCM(int trigPin, int echoPin) {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  long duration = pulseIn(echoPin, HIGH, 30000);
  if (duration == 0) return -1;
  return duration * 0.034 / 2;
}

void setup() {
  pinMode(TRIG_A, OUTPUT);
  pinMode(ECHO_A, INPUT);
  pinMode(TRIG_B, OUTPUT);
  pinMode(ECHO_B, INPUT);

  Wire.begin(21, 22);
  lcd.init();
  lcd.backlight();

  ledcAttach(BUZZER_PIN, 2000, 8); // buzzer PWM

  lcd.print("Speed Ready");
  delay(1200);
  lcd.clear();
}

void loop() {
  long distA = readDistanceCM(TRIG_A, ECHO_A);
  delay(40);
  long distB = readDistanceCM(TRIG_B, ECHO_B);

  // Detect passing Sensor A
  if (distA > 0 && distA < TRIGGER_DISTANCE && !detectedA) {
    detectedA = true;
    timeA = millis();
  }

  // Detect passing Sensor B
  if (distB > 0 && distB < TRIGGER_DISTANCE && detectedA && !detectedB) {
    detectedB = true;
    timeB = millis();
  }

  // Calculate scaled speed
  if (detectedA && detectedB) {
    float deltaTime = (timeB - timeA) / 1000.0; // seconds
    float cmPerSec = SENSOR_DISTANCE_CM / deltaTime;
    float speedMPH = cmPerSec * MPH_PER_CMPS;

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Speed:");
    lcd.print(speedMPH, 1);
    lcd.print(" MPH");

    if (speedMPH > SPEED_LIMIT_MPH) {
      lcd.setCursor(0, 1);
      lcd.print("TOO FAST!");
      ledcWrite(BUZZER_PIN, 180);
    } else {
      lcd.setCursor(0, 1);
      lcd.print("OK");
      ledcWrite(BUZZER_PIN, 0);
    }

    delay(2000); // pause to read speed

    // Reset for next pass
    detectedA = false;
    detectedB = false;
    ledcWrite(BUZZER_PIN, 0);
    lcd.clear();
  }

  delay(50);
}
