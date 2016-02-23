/**********************************************/
//Test NodeMCU and Shinyei PPD42NS
//https://github.com/opendata-stuttgart/sensors-software/blob/master/esp8266-arduino/ppd42ns-wifi-dht/ppd42ns-wifi-dht.ino
/**********************************************/

/**********************************************/
/* WiFi declarations
/**********************************************/
#include <ESP8266WiFi.h>

const char* ssid = "ThuMan";
const char* password = "MaiTraLe";

//Thingspeak Settings
const String CHANNEL_ID = "87595";
const String API_READ_KEY = "KCXKB0GWC3MLJHS9";
const char* server = "api.thingspeak.com";

// const int httpPort = 8000;
const int httpPort = 80;
int value = 0;

/**********************************************/
/* Variable Definitions for PPD24NS
/**********************************************/
// P1 for PM10 & P2 for PM25
boolean valP1 = HIGH;
boolean valP2 = HIGH;

unsigned long starttime;
unsigned long durationP1;
unsigned long durationP2;

boolean trigP1 = false;
boolean trigP2 = false;
unsigned long trigOnP1;
unsigned long trigOnP2;

unsigned long sampletime_ms = 30000;
unsigned long lowpulseoccupancyP1 = 0;
unsigned long lowpulseoccupancyP2 = 0;

float ratio = 0;
float concPM25 = 0;
float concPM10 = 0;

//void sendConcentration(float valPM25, float valPM10);

/**********************************************/
/* The Setup
/**********************************************/
void setup() {
  Serial.begin(9600); //Output to Serial at 9600 baud
  delay(10);
  pinMode(4,INPUT); // Listen at the designated PIN
  pinMode(5,INPUT); //Listen at the designated PIN
  starttime = millis(); // store the start time
  delay(10);
  connectWifi(); // Start ConnecWifi 
  Serial.print("\n"); 
  Serial.println("ChipId: ");
  Serial.println(ESP.getChipId());
}

/**********************************************/
/* And action
/**********************************************/
void loop() {
  String data;

  // Read pins connected to ppd42ns
  valP1 = digitalRead(4);
  valP2 = digitalRead(5);
  //Serial.print(valP1);
  
  if(valP1 == LOW && trigP1 == false){
    trigP1 = true;
    trigOnP1 = micros();
  }
  
  if (valP1 == HIGH && trigP1 == true){
    durationP1 = micros() - trigOnP1;
    lowpulseoccupancyP1 = lowpulseoccupancyP1 + durationP1;
    trigP1 = false;
  }
  
  if(valP2 == LOW && trigP2 == false){
    trigP2 = true;
    trigOnP2 = micros();
  }
  
  if (valP2 == HIGH && trigP2 == true){
    durationP2 = micros() - trigOnP2;
    lowpulseoccupancyP2 = lowpulseoccupancyP2 + durationP2;
    trigP2 = false;
  }

  // Checking if it is time to sample
  if ((millis()-starttime) > sampletime_ms)
  {
    ratio = lowpulseoccupancyP1/(sampletime_ms*10.0);                 // int percentage 0 to 100
    concPM10 = 1.1*pow(ratio,3)-3.8*pow(ratio,2)+520*ratio+0.62; // spec sheet curve
    // Begin printing
    Serial.print("LPO P10     : ");
    Serial.println(lowpulseoccupancyP1);
    Serial.print("Ratio PM10  : ");
    Serial.print(ratio);
    Serial.println(" %");
    Serial.print("PM10 Count  : ");
    Serial.println(concPM10);

    ratio = lowpulseoccupancyP2/(sampletime_ms*10.0);
    concPM25 = 1.1*pow(ratio,3)-3.8*pow(ratio,2)+520*ratio+0.62;
    // Begin printing
    Serial.print("LPO PM25    : ");
    Serial.println(lowpulseoccupancyP2);
    Serial.print("Ratio PM25  : ");
    Serial.print(ratio);
    Serial.println(" %");
    Serial.print("PM25 Count  : ");
    Serial.println(concPM25);
    sendConcentration(concPM25,concPM10);
    
    // Resetting for next sampling
    lowpulseoccupancyP1 = 0;
    lowpulseoccupancyP2 = 0;
    starttime = millis(); // store the start time

    Serial.println("------------------------------");
  }
}

void sendConcentration(float valPM25, float valPM10) {
  WiFiClient client;
  if (client.connect(server, 80)) {
   String postStr = API_READ_KEY;
   postStr += "&field2=";
   postStr += String(valPM25);
   postStr += "&field3=";  
   postStr += String(valPM10);

   postStr += "\r\n\r\n";
   
   client.print("POST /update HTTP/1.1\n");
   client.print("Host: api.thingspeak.com\n");
   client.print("Connection: close\n");
   client.print("X-THINGSPEAKAPIKEY: " + API_READ_KEY + "\n");
   client.print("Content-Type: application/x-www-form-urlencoded\n");
   client.print("Content-Length: ");
   client.print(postStr.length());
   client.print("\n\n");
   client.print(postStr);
   delay(1000);
  }
  else Serial.print("Connection failed");
  
  client.stop(); 
}  

/**********************************************/
/* WiFi connecting script
/**********************************************/
void connectWifi() {
  WiFi.begin(ssid, password); // Start WiFI
  
  Serial.print("Connecting ");
  while (WiFi.status() != WL_CONNECTED) 
  {
    delay(500);
    Serial.print("Connecting to Wifi");
  }
  Serial.println("WiFi connected");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}

String Float2String(float value)
{
  // Convert a float to String with two decimals.
  char temp[15];
  String s;

  dtostrf(value,13, 2, temp);
  s = String(temp);
  s.trim();
  return s;
}
