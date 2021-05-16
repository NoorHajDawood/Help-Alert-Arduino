#include <EMailSender.h>
EMailSender emailSend("trashArduino@gmail.com", "7Fdf8591");

//#include <ESP8266WiFi.h>
#include <WiFi.h>
#include "Adafruit_MQTT.h"
#include "Adafruit_MQTT_Client.h"


/********* WiFi Access Point ***********/

#define WLAN_SSID       "CovidCave"
#define WLAN_PASS       "01234567890"

/********* Adafruit.io Setup ***********/

#define AIO_SERVER      "io.adafruit.com"
#define AIO_SERVERPORT  1883                   // use 8883 for SSL
#define AIO_USERNAME    "NoorX"
#define AIO_KEY         "aio_GQmW22ChJLrqBrfzIXq176696FyN"


/**** Global State (you don't need to change this!) ******/


// Create an ESP8266 WiFiClient class to connect to the MQTT server.
WiFiClient client;
// or... use WiFiFlientSecure for SSL
//WiFiClientSecure client;

// Setup the MQTT client class by passing in the WiFi client and MQTT server and login details.
Adafruit_MQTT_Client mqtt(&client, AIO_SERVER, AIO_SERVERPORT, AIO_USERNAME, AIO_KEY);

/********** Feeds *************/

// Setup a feed called 'photocell' for publishing.
// Notice MQTT paths for AIO follow the form: <username>/feeds/<feedname>
Adafruit_MQTT_Publish light_level = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/trash-distance-front");

// Setup a feed called 'onoff' for subscribing to changes.
Adafruit_MQTT_Subscribe distance_sub = Adafruit_MQTT_Subscribe(&mqtt, AIO_USERNAME "/feeds/trash-distance-front");

/********* Sketch Code ************/

#define trigPin 27

#define echoPin 14

#define greenLED 25

#define redLED 26
int count = 0;

// Bug workaround for Arduino 1.6.6, it seems to need a function declaration
// for some reason (only affects ESP8266, likely an arduino-builder bug).
void MQTT_connect();

EMailSender::EMailMessage message;

void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println(F("Starting..."));
  delay(1000);
  Serial.println(F("\n\n##################################"));
  Serial.println(F("Help Alert"));
  Serial.println(F("Noor Haj Dawood & Bader Daka"));
  Serial.println(F("##################################"));
  
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
  Serial.println();
// SETUP ALL THE SUBSRCIPTIONS HERE 
  mqtt.subscribe(&distance_sub);     // Setup MQTT subscription for onoff feed.

pinMode(trigPin, OUTPUT);

pinMode(echoPin, INPUT);

pinMode(greenLED, OUTPUT);

pinMode(redLED, OUTPUT);
  

}




void loop() {
// We must keep this for now
  MQTT_connect();

  Serial.println(F("Reading From Cloud:"));
  Adafruit_MQTT_Subscribe *subscription;
  while ((subscription = mqtt.readSubscription(500))) {
    if (subscription == &distance_sub) {
      Serial.print(F("Got: "));
      Serial.println((char *)distance_sub.lastread);
    }
  }

  long duration; // distance;
  int distance;

digitalWrite(trigPin, LOW);

delayMicroseconds(2);

digitalWrite(trigPin, HIGH);

delayMicroseconds(10); //10

digitalWrite(trigPin, LOW);

duration = pulseIn(echoPin, HIGH);

distance = (duration/2) / 29.1;

if (distance < 24) { // This is where the LED On/Off happens

digitalWrite(greenLED,HIGH);digitalWrite(redLED,LOW); // When the Red condition is met, the Green LED should turn off digitalWrite(redLED,LOW);
light_level.publish(distance);
message.subject = "Help Your Grandpa!";
message.message = "Pico-Kit32 has detected that your grandpa is in need of your help!";
EMailSender::Response resp = emailSend.send("trashArduino@gmail.com", message);
Serial.println("Sending status: ");
Serial.println(resp.code);
Serial.println(resp.desc);
Serial.println(resp.status);
delay(5000);

} else {

digitalWrite(greenLED,LOW); digitalWrite(redLED,HIGH);
light_level.publish(0);
delay(1000);
}

if (distance >= 200 || distance <= 0){

Serial.println("Out of range");

} else {

Serial.print(distance); Serial.println(" cm");

} 
//delay(500);
  // increment the count by 1
//  if (! light_level.publish(count)) {
//    Serial.println(F("Failed"));
//  } else {
//    Serial.println(F("OK!"));
//  }

  // ping the server to keep the mqtt connection alive
  // NOT required if you are publishing once every KEEPALIVE seconds
  /*
  if(! mqtt.ping()) {
    mqtt.disconnect();
  }
  */

}




// Function to connect and reconnect as necessary to the MQTT server.
// Should be called in the loop function and it will take care if connecting.
void MQTT_connect() {
  int8_t ret;

  // Stop if already connected.
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
         // basically die and wait for WDT to reset me
         while (1);
       }
  }
  Serial.println("MQTT Connected!");
}
