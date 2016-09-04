/*  Article Node-RED Netpie                                */
/*  More information visit : http://nodered.org/ 
                             https://netpie.io             
                             http://cmmakerclub.com/       */

#include <ESP8266WiFi.h>
#include <MicroGear.h>
#include <DHT.h>

#include "CMMC_Blink.hpp"
CMMC_Blink blinker;
#include "CMMC_Interval.hpp"

const char* ssid     = "SSID";
const char* password = "PASSWORD";


#define APPID       "CMMC"
#define KEY         "xxxx"
#define SECRET      "xxxx"
#define ALIAS       "DHTdev"

WiFiClient client;
AuthClient *authclient;

#define DHTPIN 12
#define DHTTYPE DHT22   // DHT 22  (AM2302), AM2321
DHT dht(DHTPIN, DHTTYPE);
CMMC_Interval timer001;

int timer = 0;
int relayPin = 15; //control relay pin

MicroGear microgear(client);

/* If a new message arrives, do this */
void onMsghandler(char *topic, uint8_t* msg, unsigned int msglen) {
  Serial.print("Incoming message --> ");
  msg[msglen] = '\0';
  Serial.println((char *)msg);
  String msg2 = String((char*)msg);

  if (msg2 == "ON") {
    digitalWrite(relayPin, HIGH);
    digitalWrite(LED_BUILTIN, LOW);
  }
  else if (msg2 == "OFF") {
    digitalWrite(relayPin, LOW);
    digitalWrite(LED_BUILTIN, HIGH);
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
  //on led when Connected to NETPIE
  analogWrite(LED_BUILTIN, 0); //LED_BUILTIN use avctive Low to On
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

  pinMode(relayPin, OUTPUT);
  pinMode(LED_BUILTIN, OUTPUT);

  dht.begin();
  blinker.init();
  Serial.begin(115200);
  Serial.println("Starting...");

  blinker.blink(50, LED_BUILTIN);
  delay(200);

  if (WiFi.begin(ssid, password)) {
    while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(".");
    }
  }

  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  blinker.blink(200, LED_BUILTIN);
  /* Initial with KEY, SECRET and also set the ALIAS here */
  microgear.init(KEY, SECRET, ALIAS);

  /* connect to NETPIE to a specific APPID */
  microgear.connect(APPID);

  // connected to netpie so turn off the led
  blinker.detach();
  digitalWrite(LED_BUILTIN, HIGH);
}

void loop() {
  microgear.loop();
  if (microgear.connected()) {
    timer001.every_ms(2000, [&]() {
      Serial.print("Publish... ");
      //******  read DHT sensor very 2sec
      float h = 0.00f;
      float t = 0.00f;
      // Reading temperature or humidity takes about 250 milliseconds!
      // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
      h = dht.readHumidity();
      // Read temperature as Celsius (the default)
      t = dht.readTemperature();

      if (isnan(h) || isnan(t)) {
        Serial.println("Failed to read from DHT sensor!");
        //return;
      }
      else {
        Serial.print("Humidity: ");
        Serial.print(h);
        Serial.print(" %\t");
        Serial.print("Temperature: ");
        Serial.print(t);
        Serial.println(" *C ");

        /* Chat with the microgear named ALIAS which is myself */
        microgear.chat("plug001/temp", (String)t);
        microgear.chat("plug001/humid", (String)h);

        char topic_temp[MAXTOPICSIZE];
        char topic_humid[MAXTOPICSIZE];
        sprintf(topic_temp, "/gearname/%s/temp", ALIAS);
        sprintf(topic_humid, "/gearname/%s/humid", ALIAS);
        //retain message
        microgear.publish(topic_temp, String(t), true);
        microgear.publish(topic_humid, String(h), true);
      }
    });
  }
  else {
    Serial.println("DIS CONNECTED");
    microgear.connect(APPID);
  }
}
