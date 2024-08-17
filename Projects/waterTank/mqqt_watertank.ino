#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <EEPROM.h>
#include <ArduinoJson.h>

#define S0 D0                            
#define S1 D1                            
#define S2 D2                            
#define S3 D3                            
#define SIG A0                            
int EEPROM_SIZE = 512;

int tank1_rawData;                        
int tank2_rawData;                        
int tank3_rawData;                        
int tank1WaterFlow_rawData;                
int tank2WaterFlow_rawData;                
int tank3WaterFlow_rawData;                

int tank1 = 0;                          
int tank2 = 0;                            
int tank3 = 0;                            
int tank1WaterFlow = 0;                  
int tank2WaterFlow = 0;                  
int tank3WaterFlow = 0;                  

// Initializing tank map values
int tank1_min = 0;                          
int tank2_min = 0;
int tank3_min = 0;                            
int tank1_max = 0;                          
int tank2_max = 0;                          
int tank3_max = 0;

int tank1_min_addr = 0;                          
int tank2_min_addr = 10;
int tank3_min_addr = 20;                            
int tank1_max_addr = 30;                          
int tank2_max_addr = 40;                          
int tank3_max_addr = 50;

// Replace these with your WiFi and MQTT broker detail

const char* ssid = "TamilWorkshop";
const char* password = "#Tw007AS2";
const char* mqtt_server = "192.168.1.1";
const char* mqtt_username = "akashsiva";
const char* mqtt_password = "sivaA@321";

const char* subscribeTankConfig = "tank/config";
const char* publishTankConfigResult = "tank/config/result";
const char* publishTankData = "tank/data";

WiFiClient espClient;
PubSubClient client(espClient);

void setup() {
  Serial.begin(115200);
  EEPROM.begin(EEPROM_SIZE);
  readEeprom();
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
  pinMode(S0, OUTPUT);                    
  pinMode(S1, OUTPUT);                      
  pinMode(S2, OUTPUT);                      
  pinMode(S3, OUTPUT);                      
  pinMode(SIG, INPUT);                    
   
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

void updateMappingValues(String tank, int minVal, int maxVal) {
  if (tank == "Tank1") {
    tank1_min = minVal;
    tank1_max = maxVal;
  } else if (tank == "Tank2") {
    tank2_min = minVal;
    tank2_max = maxVal;
  } else if (tank == "Tank3") {
    tank3_min = minVal;
    tank3_max = maxVal;
  }
  writeEeprom();
}

void readEeprom() {
  EEPROM.get(tank1_min_addr, tank1_min);
  EEPROM.get(tank2_min_addr, tank2_min);
  EEPROM.get(tank3_min_addr, tank3_min);
  EEPROM.get(tank1_max_addr, tank1_max);
  EEPROM.get(tank2_max_addr, tank2_max);
  EEPROM.get(tank3_max_addr, tank3_max);
  Serial.println("####### EEPROM read Success ##########");
}

void writeEeprom() {
  EEPROM.put(tank1_min_addr, tank1_min);
  EEPROM.put(tank2_min_addr, tank2_min);
  EEPROM.put(tank3_min_addr, tank3_min);
  EEPROM.put(tank1_max_addr, tank1_max);
  EEPROM.put(tank2_max_addr, tank2_max);
  EEPROM.put(tank3_max_addr, tank3_max);
  EEPROM.commit();
  String payload = "{";
  payload += "\"result\": Updated successfully,";
  payload += "}";
  client.publish(publishTankConfigResult, payload.c_str());
  Serial.println("####### EEPROM write Success ##########");
}

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
 
  String message = "";
  for (int i = 0; i < length; i++) {
    message += (char)payload[i];
  }
  Serial.println(message);
 
  DynamicJsonDocument doc(256);
  DeserializationError error = deserializeJson(doc, message);

  if (error) {
    Serial.print("deserializeJson() failed: ");
    Serial.println(error.f_str());
    return;
  }

  const char* tank = doc["tank"];
  int minVal = doc["minVal"];
  int maxVal = doc["maxVal"];

  if (String(topic) == subscribeTankConfig) {
    updateMappingValues(String(tank), minVal, maxVal);
  }
}

void reconnect() {
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    if (client.connect("NodeMCUClient", mqtt_username, mqtt_password)) {
      Serial.println("connected");
      client.subscribe(subscribeTankConfig);
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  // Read sensor values
  // Channel 0 (C0 pin - binary output 0,0,0,0)
  digitalWrite(S0, LOW); digitalWrite(S1, LOW); digitalWrite(S2, LOW); digitalWrite(S3, LOW);
  delay(100);
  tank1_rawData = analogRead(SIG);
  tank1 = map(tank1_rawData, 0, 1023, 0, 100);
  delay(100);

  // Channel 1 (C1 pin - binary output 1,0,0,0)
  digitalWrite(S0, HIGH); digitalWrite(S1, LOW); digitalWrite(S2, LOW); digitalWrite(S3, LOW);
  delay(100);
  tank2_rawData = analogRead(SIG);
  tank2 = map(tank2_rawData, 0, 1023, 0, 100);
  delay(100);

  // Channel 2 (C2 pin - binary output 0,1,0,0)
  digitalWrite(S0, LOW); digitalWrite(S1, HIGH); digitalWrite(S2, LOW); digitalWrite(S3, LOW);
  tank3_rawData = analogRead(SIG);
  tank3 = map(tank3_rawData, 0, 1023, 0, 100);
  delay(100);

  // Channel 3 (C3 pin - binary output 1,1,0,0)
  digitalWrite(S0, HIGH); digitalWrite(S1, HIGH); digitalWrite(S2, LOW); digitalWrite(S3, LOW);
  tank1WaterFlow_rawData = analogRead(SIG);
  delay(100);

  // Channel 4 (C4 pin - binary output 0,0,1,0)
  digitalWrite(S0, LOW); digitalWrite(S1, LOW); digitalWrite(S2, HIGH); digitalWrite(S3, LOW);
  tank2WaterFlow_rawData = analogRead(SIG);
  delay(100);

  // Channel 5 (C5 pin - binary output 1,0,1,0)
  digitalWrite(S0, HIGH); digitalWrite(S1, LOW); digitalWrite(S2, HIGH); digitalWrite(S3, LOW);
  tank3WaterFlow_rawData = analogRead(SIG);
  delay(100);

  // Create JSON payload
  String payload = "{";
  payload += "\"tank1\":" + String(tank1) + ",";
  payload += "\"tank2\":" + String(tank2) + ",";
  payload += "\"tank3\":" + String(tank3) + ",";
  payload += "\"tank1_raw\":" + String(tank1_rawData) + ",";
  payload += "\"tank2_raw\":" + String(tank2_rawData) + ",";
  payload += "\"tank3_raw\":" + String(tank3_rawData) + ",";
  payload += "\"tankflow1\":" + String(tank1WaterFlow) + ",";
  payload += "\"tankflow2\":" + String(tank2WaterFlow) + ",";
  payload += "\"tankflow3\":" + String(tank3WaterFlow);
  payload += "}";

 

  // Publish data
  client.publish(publishTankData, payload.c_str());

  delay(2000); // Adjust as necessary for your use case
}
