/************************************************************************
  Example sketch for IKEA Krydda growth control system
  With web dashboard at io.adafruit.com
  Works with Arduino, ESP8266
  Sensors:
  BH1750FVI light sensor

  Markus Ulsass
  Hamburg, Germany
  github@tradewire.de
  23-5-2016

  Libraries needed to install:
  BH1750FVI light sensor library: https://github.com/markbeee/BH1750FVI
  Adafruit MQTT library https://github.com/adafruit/Adafruit_MQTT_Library

  Demo dashboard on adafruit io: https://io.adafruit.com/markb2
**************************************************************************/

// Libraries
#include <ESP8266WiFi.h> // ESP WiFi library
#include <Wire.h> // I2C Arduino library
#include "Adafruit_MQTT.h" // Adafruit MQTT library
#include "Adafruit_MQTT_Client.h"
#include "BH1750FVI.h" // BH1750FVI Light sensor library

// Defines
#define AIO_SERVER      "io.adafruit.com"
#define AIO_SERVERPORT  1883
#define AIO_USERNAME    "username_adafruit_io" // put in your username for adafruit.io
#define AIO_KEY         "AIO_key_adafruit_io" // put in the key for adafruit.io

// Constants
const char* ssid     = "yourSSID"; // put in your WiFi SSID
const char* password = "yourWiFiPassword"; // put in your WiFi password

// Functions
void connect(); // Function for connecting to Adafruit IO

// Create an ESP8266 WiFiClient class to connect to the MQTT server.
WiFiClient client;

// Store the MQTT server, client ID, username, and password in flash memory.
// This is required for using the Adafruit MQTT library.
const char MQTT_SERVER[] PROGMEM    = AIO_SERVER;
// Set a unique MQTT client ID using the AIO key + the date and time the sketch
// was compiled (so this should be unique across multiple devices for a user,
// alternatively you can manually set this to a GUID or other random value).
const char MQTT_CLIENTID[] PROGMEM  = AIO_KEY __DATE__ __TIME__;
const char MQTT_USERNAME[] PROGMEM  = AIO_USERNAME;
const char MQTT_PASSWORD[] PROGMEM  = AIO_KEY;

// Setup the MQTT client class by passing in the WiFi client and MQTT server and login details.
Adafruit_MQTT_Client mqtt(&client, MQTT_SERVER, AIO_SERVERPORT, MQTT_CLIENTID, MQTT_USERNAME, MQTT_PASSWORD);

/****************************** Feeds ***************************************/

const char AMBIENT_LIGHT[] PROGMEM = AIO_USERNAME "/feeds/Ambient_Light";
Adafruit_MQTT_Publish ambientLight1 = Adafruit_MQTT_Publish(&mqtt, AMBIENT_LIGHT);

BH1750FVI LightSensor;
int lux;

void setup() {

  Serial.begin(115200);

  LightSensor.begin();
  LightSensor.setMode(Continuously_High_Resolution_Mode); // see datasheet page 5 for modes

  Serial.println("Light sensor BH1750FVI found and running");

  // We start by connecting to a WiFi network
  Serial.println();
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

  // Connect to Adafruit IO
  connect();

}

void loop() {

  // ping adafruit io a few times to make sure we remain connected
  if (! mqtt.ping(3)) {
    // reconnect to adafruit io
    if (! mqtt.connected())
      connect();
  }

  lux = LightSensor.getAmbientLight();

  // Debugging
  Serial.print("Ambient light intensity: ");
  Serial.print(lux);
  Serial.println(" lux");

  // Publish data
  if (! ambientLight1.publish(lux)) {
    Serial.println(F("Failed"));
  } else {
    Serial.println(F("OK!"));
  }

  delay(10000);

}

// connect to adafruit io via MQTT
void connect() {

  Serial.print(F("Connecting to Adafruit IO... "));

  int8_t ret;

  while ((ret = mqtt.connect()) != 0) {

    switch (ret) {
      case 1: Serial.println(F("Wrong protocol")); break;
      case 2: Serial.println(F("ID rejected")); break;
      case 3: Serial.println(F("Server unavail")); break;
      case 4: Serial.println(F("Bad user/pass")); break;
      case 5: Serial.println(F("Not authed")); break;
      case 6: Serial.println(F("Failed to subscribe")); break;
      default: Serial.println(F("Connection failed")); break;
    }

    if (ret >= 0)
      mqtt.disconnect();

    Serial.println(F("Retrying connection..."));
    delay(5000);

  }

  Serial.println(F("Adafruit IO Connected!"));

}
