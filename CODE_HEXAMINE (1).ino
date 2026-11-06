




#include <WiFi.h>
#include <HTTPClient.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_ADXL345_U.h>
#include <math.h>

// ------------ WIFI SETTINGS ------------
const char* WIFI_SSID = "CLOUDLAB";
const char* WIFI_PASSWORD = "kingston123";

// ------------ THINGSPEAK SETTINGS ------------
const char* WRITE_API_KEY = "TRSDWIAEA98KKJ7Y";


// ------------ ESP32 PINS ------------
#define TRIG_PIN       5
#define ECHO_PIN       18
#define VIBRATION_PIN  19
#define BUZZER_PIN     23

#define SDA_PIN        21
#define SCL_PIN        22

// ------------ LCD AND ACCELEROMETER ------------
LiquidCrystal_I2C lcd(0x27, 16, 2);
Adafruit_ADXL345_Unified accel =
    Adafruit_ADXL345_Unified(12345);

// ------------ RISK THRESHOLDS ------------
const float WARNING_GAP_MM = 300.0;
const float DANGER_GAP_MM  = 400.0;

const float WARNING_TILT_DEG = 5.0;
const float DANGER_TILT_DEG  = 10.0;

// Change to HIGH if your vibration module
// outputs HIGH when vibration is detected.
const int VIBRATION_ACTIVE = LOW;

// ------------ SENSOR VALUES ------------
float crackGapMM = -1;
float tiltX = 0;
float tiltY = 0;
float accelerationG = 0;

int vibration = 0;
int riskLevel = 0;

unsigned long lastUpload = 0;
const unsigned long UPLOAD_INTERVAL = 20000;

unsigned long lastLCDChange = 0;
const unsigned long LCD_INTERVAL = 2000;

int lcdPage = 0;
const int TOTAL_LCD_PAGES = 5;

// ------------ READ ULTRASONIC SENSOR ------------
float readCrackGapMM() {
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(3);

  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  unsigned long duration =
      pulseIn(ECHO_PIN, HIGH, 30000);

  if (duration == 0) {
    return -1;
  }

  float distanceCM = duration * 0.0343 / 2.0;

  return distanceCM * 10.0;
}

// ------------ READ ADXL345 ------------
void readTilt() {
  sensors_event_t event;
  accel.getEvent(&event);

  float ax = event.acceleration.x;
  float ay = event.acceleration.y;
  float az = event.acceleration.z;

  accelerationG =
      sqrt(ax * ax + ay * ay + az * az) / 9.80665;

  tiltX = atan2(ax, sqrt(ay * ay + az * az))
          * 180.0 / PI;

  tiltY = atan2(ay, sqrt(ax * ax + az * az))
          * 180.0 / PI;
}

// ------------ CALCULATE RISK ------------
void calculateRisk() {
  float maximumTilt = max(abs(tiltX), abs(tiltY));

  bool danger =
      (crackGapMM >= DANGER_GAP_MM) ||
      (maximumTilt >= DANGER_TILT_DEG) ||
      (vibration == 1);

  bool warning =
      (crackGapMM >= WARNING_GAP_MM) ||
      (maximumTilt >= WARNING_TILT_DEG);

  if (danger) {
    riskLevel = 2;
  } else if (warning) {
    riskLevel = 1;
  } else {
    riskLevel = 0;
  }

  digitalWrite(BUZZER_PIN,
               riskLevel == 2 ? HIGH : LOW);
}

// ------------ DISPLAY ON 16x2 LCD ------------
void showLCD() {
  // Cycle through sensor readings every 2 seconds
  if (millis() - lastLCDChange >= LCD_INTERVAL) {
    lastLCDChange = millis();
    lcdPage = (lcdPage + 1) % TOTAL_LCD_PAGES;
  }

  String line1;

  switch (lcdPage) {
    case 0:
      if (crackGapMM < 0) {
        line1 = "Gap: Sensor Error";
      } else {
        line1 = "Gap:" + String(crackGapMM, 1) + "mm";
      }
      break;

    case 1:
      line1 = vibration
              ? "Vibration: ON"
              : "Vibration: OFF";
      break;

    case 2:
      line1 = "Tilt X:" + String(tiltX, 1);
      break;

    case 3:
      line1 = "Tilt Y:" + String(tiltY, 1);
      break;

    case 4:
      line1 = "Accel:" +
              String(accelerationG, 2) + "g";
      break;
  }

  // Print Line 1 and clear any remaining characters
  lcd.setCursor(0, 0);

  for (int i = 0; i < 16; i++) {
    if (i < line1.length()) {
      lcd.print(line1[i]);
    } else {
      lcd.print(' ');
    }
  }

  // Line 2: always display current status
  String line2;

  if (riskLevel == 2) {
    line2 = "State: DANGER";
  } else if (riskLevel == 1) {
    line2 = "State: WARNING";
  } else {
    line2 = "State: NORMAL";
  }

  lcd.setCursor(0, 1);

  for (int i = 0; i < 16; i++) {
    if (i < line2.length()) {
      lcd.print(line2[i]);
    } else {
      lcd.print(' ');
    }
  }
}

// ------------ UPLOAD TO THINGSPEAK ------------
void uploadToThingSpeak() {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("WiFi disconnected");
    return;
  }

  HTTPClient http;

  String url =
      "http://api.thingspeak.com/update?api_key=" +
      String(WRITE_API_KEY) +
      "&field1=" + String(crackGapMM, 2) +
      "&field2=" + String(vibration) +
      "&field3=" + String(tiltX, 2) +
      "&field4=" + String(tiltY, 2) +
      "&field5=" + String(accelerationG, 3) +
      "&field6=" + String(riskLevel);

  http.begin(url);

  int responseCode = http.GET();

  if (responseCode > 0) {
    String response = http.getString();

    if (response.toInt() > 0) {
      Serial.println("ThingSpeak upload successful");
      Serial.println("Entry ID: " + response);
    } else {
      Serial.println("ThingSpeak update rejected");
    }
  } else {
    Serial.print("Upload error: ");
    Serial.println(responseCode);
  }

  http.end();
}

// ------------ SETUP ------------
void setup() {
  Serial.begin(115200);

  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  pinMode(VIBRATION_PIN, INPUT);
  pinMode(BUZZER_PIN, OUTPUT);

  digitalWrite(TRIG_PIN, LOW);
  digitalWrite(BUZZER_PIN, LOW);

  Wire.begin(SDA_PIN, SCL_PIN);

  lcd.init();
  lcd.backlight();

  lcd.setCursor(0, 0);
  lcd.print("HEXAMINE");
  lcd.setCursor(0, 1);
  lcd.print("Initializing...");

  if (!accel.begin()) {
    Serial.println("ADXL345 not detected");

    lcd.clear();
    lcd.print("ADXL345 ERROR");

    while (true) {
      delay(1000);
    }
  }

  accel.setRange(ADXL345_RANGE_4_G);

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Connecting WiFi");

  unsigned long startTime = millis();

  while (WiFi.status() != WL_CONNECTED &&
         millis() - startTime < 20000) {
    delay(500);
    Serial.print(".");
  }

  lcd.clear();

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nWiFi connected");
    Serial.println(WiFi.localIP());

  delay(500);
    lcd.setCursor(0, 0);
    lcd.print("WiFi Connected");
  } else {
    Serial.println("\nWiFi connection failed");

    lcd.setCursor(0, 0);
    lcd.print("WiFi Offline");
  }

  delay(1000);

  // Allow the first ThingSpeak upload after 20 seconds
  lastUpload = millis();
  lastLCDChange = millis();
}

// ------------ MAIN LOOP ------------
void loop() {
  // Read sensors
  crackGapMM = readCrackGapMM();

  vibration =
      (digitalRead(VIBRATION_PIN) == VIBRATION_ACTIVE)
      ? 1 : 0;

  readTilt();

  // Determine current risk state
  calculateRisk();

  // Update LCD
  showLCD();

  // Serial Monitor output
  Serial.println("--------------------");

  Serial.print("Crack gap (mm): ");
  Serial.println(crackGapMM);
  
  delay(500);

  Serial.print("Vibration: ");
  Serial.println(vibration);

  delay(500);
  Serial.print("Tilt X: ");
  Serial.println(tiltX);

  delay(500);
  Serial.print("Tilt Y: ");
  Serial.println(tiltY);

  delay(500);
  Serial.print("Acceleration (g): ");
  Serial.println(accelerationG);

  delay(500);
  Serial.print("Risk level: ");
  Serial.println(riskLevel);

  delay(500);
  // Upload to ThingSpeak every 20 seconds
  if (millis() - lastUpload >= UPLOAD_INTERVAL) {
    lastUpload = millis();
    uploadToThingSpeak();
  }

  delay(500);
}