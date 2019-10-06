#include "ESP8266WiFi.h"
#include <PubSubClient.h>
#include <time.h>


// Update these with values suitable for your network.
const char* ssid = "ESCRITORIO";
const char* password = "xxx";
const char* mqtt_server = "192.168.1.18";
#define mqtt_port 1883
#define MQTT_USER ""
#define MQTT_PASSWORD ""
#define MQTT_PUBLISH "home/pir1"
#define MQTT_RECEIVER "home/pir1"
int receivedData = 0;
int receivedData_Before ;
char* hora_e_data;


//Inputs and outputs
int analogPin = A0; //not used yet
int WiFiPin = 12; // WiFi on pin
int MQTTPin = 14; // MQTT on pin
int AlarmPin = 5; // Alarm on pin
int PirInputPin = 13; // Pir input, using pull-down resistor in the PIR NC

int pir = 0;                    // the pir initial value



WiFiClient wifiClient;

PubSubClient client(wifiClient);

void setup_wifi() {
    delay(10);
    // We start by connecting to a WiFi network
    Serial.println();
    Serial.print("Connecting to ");
    Serial.println(ssid);
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(".");
    }
    randomSeed(micros());
    Serial.println("");
    Serial.println("WiFi connected");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());
    configTime(1 * 3600, 1, "pool.ntp.org", "time.nist.gov");
    Serial.println("\nWaiting for time");
    while (!time(nullptr)) {
      Serial.print(".");
      delay(500);
    }
    //time_t now = time(nullptr);
    //Serial.println(ctime(&now));
    digitalWrite(WiFiPin, HIGH); // sets the digital pin on
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = "ESP32Client-";
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    if (client.connect(clientId.c_str(),MQTT_USER,MQTT_PASSWORD)) {
      Serial.println("connected");
      //Once connected, publish an announcement...
      client.publish(MQTT_PUBLISH, "online");
      Serial.println("MQTT connection up");
      // ... and resubscribe
      client.subscribe(MQTT_RECEIVER);
      digitalWrite(MQTTPin, HIGH); // sets the digital pin on
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      digitalWrite(MQTTPin, LOW); // sets the digital pin off
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void callback(char* topic, byte *payload, unsigned int length) {
    //Serial.println("-------new message from broker-----");
    //Serial.print("channel:");
    //Serial.println(topic);
    //Serial.print("data:");  
    Serial.write(payload, length);
    //Serial.println();
    receivedData = payload[0]-48;    
}

void setup() {
  Serial.begin(115200);
  pinMode(WiFiPin, OUTPUT);
  pinMode(MQTTPin, OUTPUT);
  pinMode(AlarmPin, OUTPUT);
  digitalWrite(WiFiPin, LOW); // sets the digital pin off
  digitalWrite(MQTTPin, LOW); // sets the digital pin off
  digitalWrite(AlarmPin, LOW); // sets the digital pin off
  pinMode(PirInputPin, INPUT);  
  Serial.setTimeout(500);// Set time out for 
  setup_wifi();
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);
  reconnect();
}

void publishSerialData(char *serialData){
  if (!client.connected()) {
    reconnect();
  }
  client.publish(MQTT_PUBLISH, serialData);
}



void loop() {
   client.loop();

   pir = digitalRead(PirInputPin);
   if (pir == 0 ) {
       time_t now = time(nullptr);
       //Serial.println(ctime(&now));
       hora_e_data = ctime(&now);
       client.publish(MQTT_PUBLISH, hora_e_data);
       digitalWrite(AlarmPin, HIGH); // sets the digital pin on
   }
   delay(2000);        // delay in between reads for stability and to reduce MQTT messages
   digitalWrite(AlarmPin, LOW); // sets the digital pin off
   
 }
