#include <ArduinoJson.h>
#include <Arduino.h>
#include <FirebaseESP8266.h>
#include <addons/TokenHelper.h>
#include <time.h>

int fullTankDistance = 20;
int triggerPer = 20;


int t1_TRIG = 18;
int t1_ECHO = 19;
int t2_TRIG = 21;
int t2_ECHO = 22;
int t3_TRIG = 23;
int t3_ECHO = 25;

int t1_min = 0;
int t1_max = 0;
int t2_min = 0;
int t2_max = 0;
int t3_min = 0;
int t3_max = 0;

float duration;
float distance;
int waterLevelPer;
int maxvalue = 600;

// Time relateed config
const char* ntpServer = "pool.ntp.org";
const long gmtOffset_sec = 5.5 * 3600;  // GMT offset for IST (5 hours 30 minutes)
const int daylightOffset_sec = 0;       // Chennai does not observe daylight saving time
char currentTime[50];


// Firebase config:
#define API_KEY "AIzaSyC1HHrJGbAr4RE16U34MPJZsJR_vLlV5GA"
#define DATABASE_URL "https://water-ultrasonic-default-rtdb.firebaseio.com/"
#define USER_EMAIL "arunaakash39@gmail.com"
#define USER_PASSWORD "@321"

const char* ssid = "TamilWorkshop";
const char* password = "#Tw007AS2";

// Define Firebase Data object
FirebaseData fbdo;

FirebaseAuth auth;
FirebaseConfig config;

FirebaseJson fetchFirebaseJson(const String& path) {
  FirebaseData firebaseData;
  FirebaseJson jsonData;  // Initialize the FirebaseJson object to return

  if (Firebase.getJSON(firebaseData, path)) {
    if (firebaseData.dataType() == "json") {
      jsonData = firebaseData.jsonObject();  // Get the JSON object

      // Check if the JSON object contains any data
      if (jsonData.iteratorBegin() > 0) {
        FirebaseJsonData jsonDataResult;  // To hold the JSON value result

        // Extract specific values from the JSON
        if (jsonData.get(jsonDataResult, "t1_min")) {
          String t1_min = jsonDataResult.stringValue;
          Serial.println("t1_min: " + t1_min);
        }

        if (jsonData.get(jsonDataResult, "t1_max")) {
          String t1_max = jsonDataResult.stringValue;
          Serial.println("t1_max: " + t1_max);
        }

        if (jsonData.get(jsonDataResult, "t2_min")) {
          String t2_min = jsonDataResult.stringValue;
          Serial.println("t2_min: " + t2_min);
        }

        if (jsonData.get(jsonDataResult, "t2_max")) {
          String t2_max = jsonDataResult.stringValue;
          Serial.println("t2_max: " + t2_max);
        }

        if (jsonData.get(jsonDataResult, "t3_min")) {
          String t3_min = jsonDataResult.stringValue;
          Serial.println("t3_min: " + t3_min);
        }

        if (jsonData.get(jsonDataResult, "t3_max")) {
          String t3_max = jsonDataResult.stringValue;
          Serial.println("t3_max: " + t3_max);
        }

      } else {
        Serial.println("No data found in the JSON object.");
      }
    } else {
      Serial.println("Error: Data type is not JSON.");
    }
  } else {
    Serial.print("Error fetching JSON data: ");
    Serial.println(firebaseData.errorReason());
  }

  return jsonData;  // Return the JSON object, even if it's empty
}


float getvalues(int& trig, int& echo) {
  for (int i = 0; i < 20; i++) {
    digitalWrite(trig, LOW);
    delayMicroseconds(5);
    digitalWrite(trig, HIGH);
    delayMicroseconds(25);
    digitalWrite(trig, LOW);
    duration = pulseIn(echo, HIGH);
    distance = ((duration / 2) * 0.343) / 10;
    Serial.print(distance);
    Serial.print("-");
    if (distance > 0 && distance < maxvalue) {
      return distance;
    }
  }
  return 0;
}


void setup() {
  Serial.begin(9600);
  pinMode(t1_ECHO, INPUT);
  pinMode(t1_TRIG, OUTPUT);
  pinMode(t2_ECHO, INPUT);
  pinMode(t2_TRIG, OUTPUT);
  pinMode(t3_ECHO, INPUT);
  pinMode(t3_TRIG, OUTPUT);
  setup_wifi();
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  Serial.printf("Firebase Client v%s\n\n", FIREBASE_CLIENT_VERSION);
  config.api_key = API_KEY;
  auth.user.email = USER_EMAIL;
  auth.user.password = USER_PASSWORD;
  config.database_url = DATABASE_URL;
  config.token_status_callback = tokenStatusCallback;  // see addons/TokenHelper.h
  Firebase.reconnectNetwork(true);
  fbdo.setBSSLBufferSize(4096 /* Rx buffer size in bytes from 512 - 16384 */, 1024 /* Tx buffer size in bytes from 512 - 16384 */);
  Firebase.begin(&config, &auth);
  Firebase.setDoubleDigits(5);

  fetchFirebaseJson("/mapData");
}

void setup_wifi() {
  delay(10);
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void ensureWifiConnected() {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("WiFi disconnected. Attempting to reconnect...");
    WiFi.begin(ssid, password);
    int attempt = 0;
    while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(".");
      attempt++;
      if (attempt > 60) {  // try for 30 seconds
        Serial.println("Failed to reconnect to WiFi. Restarting...");
        ESP.restart();
      }
    }
    Serial.println("");
    Serial.println("WiFi reconnected");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());
  }
}

void reconnectFirebase() {
  Serial.print("Attempting Firebase connection...");
  Firebase.reconnectWiFi(true);
  if (Firebase.ready()) {
    Serial.println("Firebase connected");
  } else {
    Serial.println("Firebase not connected");
  }
}

void loop() {
  ensureWifiConnected();
  Serial.println("Reading sensor data 1");
  float tank1_distance = getvalues(t1_TRIG, t1_ECHO);
  int tank1 = map((int)tank1_distance, t1_min, t1_max, 0, 100);
  Serial.println("Reading sensor data 2");
  float tank2_distance = getvalues(t2_TRIG, t2_ECHO);
  int tank2 = map((int)tank2_distance, t2_min, t2_max, 0, 100);
  Serial.println("Reading sensor data 3");
  float tank3_distance = getvalues(t3_TRIG, t3_ECHO);
  int tank3 = map((int)tank3_distance, t3_min, t3_max, 0, 100);
  Serial.println("tank1: " + String(tank1_distance) + " cm");
  Serial.println("tank2: " + String(tank2_distance) + " cm");
  Serial.println("tank3: " + String(tank3_distance) + " cm");
  Serial.println("*******************************************");

  time_t now = time(nullptr);
  if (now > 0) {
    strftime(currentTime, sizeof(currentTime), "%Y.%m.%d--%H-%M-%S", localtime(&now));

    Serial.print("Current time in Chennai: ");
    Serial.println(currentTime);
  }

  FirebaseJson json;
  json.set("/tank1", tank1);
    json.set("/tank2", tank2);
    json.set("/tank3", tank3);
    json.set("/tankRawData1", tank1_distance);
    json.set("/tankRawData2", tank2_distance);
    json.set("/tankRawData3", tank3_distance);
    json.set("/tankWaterFlow1", "Tank_filling");
    json.set("/tankWaterFlow2", false);
    json.set("/tankWaterFlow3", "Tank_filling");
    json.set("/lastUpdatedTime", currentTime);


  String payload;
  json.toString(payload, true);

  // Firebase section
  if (Firebase.ready()) {

    Serial.print("Firebase sending data...");
    if (Firebase.set(fbdo, F("/sensor"), json)) {
      Serial.println("Data set successfully");
    } else {
      Serial.println(fbdo.errorReason());
    }
  } else {
    reconnectFirebase();
  }
  //  tankData.waterLevelPer = map((int)distance, emptyTankDistance, fullTankDistance, 0, 100);
}
