#include <ESP8266WiFi.h>
#include "Adafruit_MQTT.h"
#include "Adafruit_MQTT_Client.h"

#define LED_PIN 14
#define SOUND_SENSOR_PIN A0

// 0 -> 51db
// 2 -> 67db
// 3 -> 79db
// 10 -> 89db

#define WLAN_SSID       "Camila"
#define WLAN_PASS       "36982625"

#define AIO_SERVER      "io.adafruit.com"
#define AIO_SERVERPORT  1883                   // use 8883 for SSL
#define AIO_USERNAME  "mpereirajf"
#define AIO_KEY       "aio_UUFy87DeFHNfdZCGpH3o7jqTydza"

WiFiClient client;

Adafruit_MQTT_Client mqtt(&client, AIO_SERVER, AIO_SERVERPORT, AIO_USERNAME, AIO_KEY);

Adafruit_MQTT_Publish soundLevel = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/soundlevel");
Adafruit_MQTT_Subscribe maxSoundLevel = Adafruit_MQTT_Subscribe(&mqtt, AIO_USERNAME "/feeds/maxsoundlevel");

void MQTT_connect();
float maxSoundLevelF = 90;

void setup() {
  Serial.begin(9600);
  delay(10);
  pinMode(LED_PIN, OUTPUT);
  pinMode(SOUND_SENSOR_PIN, INPUT);

  // Connect to WiFi access point.
  Serial.println(); Serial.println();
  Serial.print("Connecting to ");
  Serial.println(WLAN_SSID);

  WiFi.begin(WLAN_SSID, WLAN_PASS);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println();

  Serial.println("WiFi connected");
  Serial.println("IP address: "); Serial.println(WiFi.localIP());

  mqtt.subscribe(&maxSoundLevel);
}

uint32_t x=0;

void loop() {
  MQTT_connect();

  Adafruit_MQTT_Subscribe *subscription;
  while ((subscription = mqtt.readSubscription(5000))) {
    if (subscription == &maxSoundLevel) {
      String str = "";
      for(uint8_t i = 0; i < maxSoundLevel.datalen; i++) {
        str += String(maxSoundLevel.lastread[i] - 48);
      }
      maxSoundLevelF = str.toFloat();
      Serial.print(F("Got: "));
      Serial.println((char *)maxSoundLevel.lastread);
    }
  }

  int dif = analogRead(SOUND_SENSOR_PIN) - 565;
  dif = abs(dif);

  Serial.println(analogRead(SOUND_SENSOR_PIN));

  float db = (dif == 0 ? 0 : 42.0 * log10(dif)) + 50.0;  
  Serial.print("Sound Level: ");
  Serial.println(db);
  Serial.print("maxSoundLevelF: ");
  Serial.println(maxSoundLevelF);
  if (db > maxSoundLevelF) {
    digitalWrite(LED_PIN, HIGH);
  } else {
    digitalWrite(LED_PIN, LOW);
  }


  static float last = 0;
  float now = millis();
  if (now - last > 5000) {
    last = now;
    if (! soundLevel.publish(db)) {
      Serial.println(F("Failed"));
    } else {
      Serial.println(F("OK!"));
    }
  }
  delay(1);
}

void MQTT_connect() {
  int8_t ret;

  if (mqtt.connected()) {
    return;
  }

  Serial.print("Connecting to MQTT... ");

  uint8_t retries = 3;
  while ((ret = mqtt.connect()) != 0) { // connect will return 0 for connected
       Serial.println(mqtt.connectErrorString(ret));
       Serial.println("Retrying MQTT connection in 5 seconds...");
       mqtt.disconnect();
       delay(5000);  // wait 5 seconds
       retries--;
       if (retries == 0) {
         while (1);
       }
  }
  Serial.println("MQTT Connected!");
}
