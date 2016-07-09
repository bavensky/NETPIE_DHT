/*  NETPIE ESP8266 basic sample                            */
/*  More information visit : https://netpie.io             */

#include <ESP8266WiFi.h>
#include <MicroGear.h>
#include "DHT.h"

const char* ssid     = "xxxxxxxx";  // Change your ssid wifi 
const char* password = "xxxxxxxx";  // Change your password wifi

// NETPIE.io : NETPIE_01
#define APPID   "HelloNETPIE"       // Change your appID
#define KEY     "xxxxxxxxxxx"       // Change your Key
#define SECRET  "xxxxxxxxxxx"       // Change your SECRET
#define ALIAS   "NETPIEname"        // Change your name

#define LEDPIN 2
#define DHTPIN 12
#define DHTTYPE DHT22

WiFiClient client;
AuthClient *authclient;
DHT dht(DHTPIN, DHTTYPE);

int timer = 0;
MicroGear microgear(client);

/* If a new message arrives, do this */
void onMsghandler(char *topic, uint8_t* msg, unsigned int msglen) {
  Serial.print("Recieve from dashboard = ");
  char Str_msg[msglen];
  for (int i = 0; i < msglen; i++)
  {
    Str_msg[i] = (char)msg[i];
    Serial.print((char)msg[i]);
  }

  String msgIN = String(Str_msg).substring(0, msglen);
  Serial.print("\nmsg from ");
  Serial.println(topic);

  if (msgIN == "ON")
  {
    digitalWrite(LEDPIN, LOW);
    delay(10);
  }
  else if (msgIN == "OFF")
  {
    digitalWrite(LEDPIN, HIGH);
    delay(10);
  }
}

void onFoundgear(char *attribute, uint8_t* msg, unsigned int msglen) {
  Serial.print("Found new member --> ");
  for (int i = 0; i < msglen; i++)
    Serial.print((char)msg[i]);
  Serial.println();
}

void onLostgear(char *attribute, uint8_t* msg, unsigned int msglen) {
  Serial.print("Lost member --> ");
  for (int i = 0; i < msglen; i++)
    Serial.print((char)msg[i]);
  Serial.println();
}

/* When a microgear is connected, do this */
void onConnected(char *attribute, uint8_t* msg, unsigned int msglen) {
  Serial.println("Connected to NETPIE...");
  /* Set the alias of this microgear ALIAS */
  microgear.setName(ALIAS);
}


void setup() {
  /* Add Event listeners */

  /* Call onMsghandler() when new message arraives */
  microgear.on(MESSAGE, onMsghandler);

  /* Call onFoundgear() when new gear appear */
  microgear.on(PRESENT, onFoundgear);

  /* Call onLostgear() when some gear goes offline */
  microgear.on(ABSENT, onLostgear);

  /* Call onConnected() when NETPIE connection is established */
  microgear.on(CONNECTED, onConnected);

  Serial.begin(115200);
  Serial.println("Starting...");

  /* Initial WIFI, this is just a basic method to configure WIFI on ESP8266.                       */
  /* You may want to use other method that is more complicated, but provide better user experience */
  if (WiFi.begin(ssid, password)) {
    while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(".");
    }
  }

  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  dht.begin();
  pinMode(LEDPIN, OUTPUT);

  /* Initial with KEY, SECRET and also set the ALIAS here */
  microgear.init(KEY, SECRET, ALIAS);

  /* connect to NETPIE to a specific APPID */
  microgear.connect(APPID);
}

void loop() {
  /* To check if the microgear is still connected */
  if (microgear.connected()) {
    Serial.println("connected");

    /* Call this method regularly otherwise the connection may be lost */
    microgear.loop();

    // อ่านค่าจากเซ็นเซอร์ DHt22
    float t = dht.readTemperature();
    float h = dht.readHumidity();

    // ส่งค่าไปยัง netpie
    microgear.chat("NETPIEname/Temperature", String(t));
    microgear.chat("NETPIEname/Humidity", String(h));

    Serial.print("Temperature = ");
    Serial.print(t);
    Serial.print("\t");
    Serial.print("Humidity = ");
    Serial.println(h);
    delay(500);
  }
  else {
    Serial.println("connection lost, reconnect...");
    microgear.connect(APPID);
  }
  delay(100);
}
