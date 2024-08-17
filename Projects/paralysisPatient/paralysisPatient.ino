#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <FirebaseESP8266.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <Wire.h>

Adafruit_MPU6050 mpu;

// Provide the token generation process info.
#include <addons/TokenHelper.h>

// Provide the RTDB payload printing info and other helper functions.
#include <addons/RTDBHelper.h>

/* 1. Define the WiFi credentials */
#define WIFI_SSID "project"
#define WIFI_PASSWORD "123123123"

// For the following credentials, see examples/Authentications/SignInAsUser/EmailPassword/EmailPassword.ino

/* 2. Define the API Key */
#define API_KEY "AIzaSyAo5xiLkk6Xuhm6HuvqW-7ma8ad7Txh9PM"

/* 3. Define the RTDB URL */
#define DATABASE_URL "https://patient-b353d-default-rtdb.firebaseio.com/" //<databaseName>.firebaseio.com or <databaseName>.<region>.firebasedatabase.app

/* 4. Define the user Email and password that alreadey registerd or added in your project */
#define USER_EMAIL "ashwiniprasad100704@gmail.com"
#define USER_PASSWORD "Ashwini@100704"

// Define Firebase Data object
FirebaseData fbdo;

FirebaseAuth auth;
FirebaseConfig config;

unsigned long sendDataPrevMillis = 0;

unsigned long count = 0;

void setup()
{


  Serial.begin(115200);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    if (!mpu.begin()) {
    Serial.println("Failed to find MPU6050 chip");
    while (1) {
      delay(10);
    }
  }
  Serial.println("MPU6050 Found!");

  Serial.print("Connecting to Wi-Fi");
  unsigned long ms = millis();
  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.print(".");
    delay(300);
  }
  Serial.println();
  Serial.print("Connected with IP: ");
  Serial.println(WiFi.localIP());
  Serial.println();

  Serial.printf("Firebase Client v%s\n\n", FIREBASE_CLIENT_VERSION);

  /* Assign the api key (required) */
  config.api_key = API_KEY;

  /* Assign the user sign in credentials */
  auth.user.email = USER_EMAIL;
  auth.user.password = USER_PASSWORD;

  /* Assign the RTDB URL (required) */
  config.database_url = DATABASE_URL;

  config.token_status_callback = tokenStatusCallback; // see addons/TokenHelper.h

  Firebase.reconnectNetwork(true);

  fbdo.setBSSLBufferSize(4096 /* Rx buffer size in bytes from 512 - 16384 */, 1024 /* Tx buffer size in bytes from 512 - 16384 */);

  Firebase.begin(&config, &auth);

  Firebase.setDoubleDigits(5);


  mpu.setAccelerometerRange(MPU6050_RANGE_2_G);
  mpu.setGyroRange(MPU6050_RANGE_250_DEG);
  mpu.setFilterBandwidth(MPU6050_BAND_5_HZ);
  Serial.print("Filter bandwidth set to: ");

  Serial.println("");
  delay(100);

}

void loop()
{

  // Firebase.ready() should be called repeatedly to handle authentication tasks.
  sensors_event_t a, g, temp;
  mpu.getEvent(&a, &g, &temp);
  if (Firebase.ready())
  {
    sendDataPrevMillis = millis();
  if (g.gyro.x > 1) {
    Serial.printf("Set int... %s\n", Firebase.setInt(fbdo, F("/data"), 1) ? "ok" : fbdo.errorReason().c_str());
  } else if (g.gyro.x < -1) {
    Serial.printf("Set int... %s\n", Firebase.setInt(fbdo, F("/data"), 2) ? "ok" : fbdo.errorReason().c_str());
  } else if (g.gyro.y > 1) {
    Serial.printf("Set int... %s\n", Firebase.setInt(fbdo, F("/data"), 3) ? "ok" : fbdo.errorReason().c_str());
  } else if (g.gyro.y < -1) {
    Serial.printf("Set int... %s\n", Firebase.setInt(fbdo, F("/data"), 4) ? "ok" : fbdo.errorReason().c_str());
  }

    Serial.print(g.gyro.x);
    Serial.print(", Y: ");
    Serial.print(g.gyro.y);
    Serial.print(", Z: ");
    Serial.print(g.gyro.z);
    Serial.println(" rad/s");


    
    
  }
}
