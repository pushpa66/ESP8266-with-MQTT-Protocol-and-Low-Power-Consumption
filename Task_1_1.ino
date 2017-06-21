
#include <ESP8266WiFi.h>
#include <PubSubClient.h>

//const char* ssid = "General RandD4G";
//const char* password = "thomas!234";
const char* ssid = "Dialog 4G";
const char* password = "1Kumara@@";
const char* mqtt_server = "development.enetlk.com";

#include "DHT.h"

#define DHTPIN D1
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);
const int sleepTime = 2;
const int t = 1000000;
const int deepSleepTime = sleepTime * t;

WiFiClient espClient;
PubSubClient client(espClient);

struct {
  float h;
  float t;
  int count = 0;
} rtcData;

void ledBlink() {
  pinMode(D4, OUTPUT);
  digitalWrite(D4, HIGH);
  delay(500);
  digitalWrite(D4, LOW);
}

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
}

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = "ESP8266Client-";
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    if (client.connect(clientId.c_str())) {
      Serial.println("connected");
      // Once connected, publish an announcement...
      //client.publish("outTopic", "hello world");
      // ... and resubscribe
      //client.subscribe("inTopic");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" Try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void transferData(char* t_val, char* h_val) {
  setup_wifi();
  client.setServer(mqtt_server, 1884);
  //  client.setCallback(callback);

  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  Serial.println("Publish message: ");
  char * topic_T = "001/randd-test/temp";
  char * topic_H = "001/randd-test/humidity";
  client.publish(topic_T, t_val);
  Serial.print(topic_T);
  Serial.print("    :");
  Serial.println(t_val);
  Serial.print(topic_H);
  Serial.print(":");
  Serial.println(h_val);
  client.publish(topic_H, h_val);
}

void rtcReset() {
  rtcData.t = 0;
  rtcData.h = 0;
  rtcData.count = 0;
  ESP.rtcUserMemoryWrite(0, (uint32_t*)&rtcData, sizeof(rtcData));
}
void setup() {
  ledBlink();
  Serial.begin(115200);
  Serial.setTimeout(2000);

  while (!Serial) {}
  
  Serial.println("DHT22 test!");
  dht.begin();
  float h = dht.readHumidity();
  float t = dht.readTemperature();
  if (isnan(h) || isnan(t)) {
    Serial.println("Failed to read from DHT sensor!");
    return;
  }

  ESP.rtcUserMemoryRead(0, (uint32_t*)&rtcData, sizeof(rtcData));
  float T = rtcData.t + t;
  float H = rtcData.h + h;
  int count = rtcData.count + 1;

  if (count < 0) {
    rtcReset();
    T = 0;
    H = 0;
    count = 0;
    Serial.println("Error is fixed");
  }

  if (count == 15) {
    float averageT = T / 15;
    float averageH = H / 15;
    T = 0;
    H = 0;
    count = 0;

    String t = String(averageT);
    char t_val[10];
    t.toCharArray(t_val, sizeof t_val);

    String h = String(averageH);
    char h_val[10];
    h.toCharArray(h_val, sizeof h_val);

    transferData(t_val, h_val);
  }
  rtcReset();
  rtcData.t = T;
  rtcData.h = H;
  rtcData.count = count;
  ESP.rtcUserMemoryWrite(0, (uint32_t*)&rtcData, sizeof(rtcData));
  Serial.print("Humidity    :");
  Serial.println(rtcData.h);
  Serial.print("Temperature :");
  Serial.println(rtcData.t);
  Serial.print("Counter     :");
  Serial.println(rtcData.count);


  Serial.print("Humidity: ");
  Serial.print(h);
  Serial.print(" %\t");
  Serial.print("Temperature: ");
  Serial.print(t);
  Serial.print(" *C ");
  Serial.println();

  if(count == 14){
    ESP.deepSleep(deepSleepTime);
  }else{
    ESP.deepSleep(deepSleepTime, WAKE_RF_DISABLED);
  }
}

void loop() {

}
