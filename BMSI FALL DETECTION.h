#include <Wire.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <math.h>

Adafruit_MPU6050 mpu;

// Pin connections
const int BUZZER_PIN = 8;
const int LED_PIN = 9;
const int BUTTON_PIN = 7;

// Fall detection thresholds
const float FREE_FALL_THRESHOLD = 3.0;
const float IMPACT_THRESHOLD = 25.0;
const float ROTATION_THRESHOLD = 2.5;

// Alarm state
bool alarmActive = false;
bool alarmSilenced = false;

unsigned long fallTime = 0;
unsigned long lastButtonTime = 0;

float previousAngleX = 0;
float previousAngleY = 0;

void setup() {
  Serial.begin(9600);
  Wire.begin();

  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(LED_PIN, OUTPUT);
  pinMode(BUTTON_PIN, INPUT_PULLUP);

  digitalWrite(BUZZER_PIN, LOW);
  digitalWrite(LED_PIN, LOW);

  Serial.println("Fall Detection System Starting...");

  if (!mpu.begin()) {
    Serial.println("ERROR: MPU6050 not found!");
    while (true) {
      digitalWrite(LED_PIN, HIGH);
      delay(200);
      digitalWrite(LED_PIN, LOW);
      delay(200);
    }
  }

  mpu.setAccelerometerRange(MPU6050_RANGE_8_G);
  mpu.setGyroRange(MPU6050_RANGE_500_DEG);
  mpu.setFilterBandwidth(MPU6050_BAND_21_HZ);

  Serial.println("MPU6050 connected successfully!");
  Serial.println("System Ready");
}

void loop() {
  sensors_event_t accel, gyro, temp;
  mpu.getEvent(&accel, &gyro, &temp);

  // Calculate acceleration magnitude in m/s^2
  float ax = accel.acceleration.x;
  float ay = accel.acceleration.y;
  float az = accel.acceleration.z;

  float magnitude = sqrt(ax * ax + ay * ay + az * az);

  // Gyroscope rotation magnitude in radians/second
  float gx = gyro.gyro.x;
  float gy = gyro.gyro.y;
  float gz = gyro.gyro.z;

  float rotation = sqrt(gx * gx + gy * gy + gz * gz);

  Serial.print("Acceleration: ");
  Serial.print(magnitude);
  Serial.print(" m/s^2 | Rotation: ");
  Serial.println(rotation);

  // Detect a possible fall
  if (!alarmActive &&
      (magnitude < FREE_FALL_THRESHOLD ||
       magnitude > IMPACT_THRESHOLD ||
       rotation > ROTATION_THRESHOLD)) {

    alarmActive = true;
    alarmSilenced = false;
    fallTime = millis();

    Serial.println("WARNING: POSSIBLE FALL DETECTED!");
  }

  // Push button silences the alarm
  if (digitalRead(BUTTON_PIN) == LOW &&
      millis() - lastButtonTime > 300) {

    lastButtonTime = millis();

    if (alarmActive) {
      alarmSilenced = true;
      Serial.println("Alarm silenced by button.");
    }
  }

  // Alarm operation
  if (alarmActive && !alarmSilenced) {
    digitalWrite(LED_PIN, HIGH);
    digitalWrite(BUZZER_PIN, HIGH);
  } else {
    digitalWrite(LED_PIN, LOW);
    digitalWrite(BUZZER_PIN, LOW);
  }

  // Automatically reset after 10 seconds
  if (alarmActive && millis() - fallTime > 10000) {
    alarmActive = false;
    alarmSilenced = false;
    Serial.println("System reset. Monitoring continues.");
  }

  delay(100);
}