#include "Adafruit_LEDBackpack.h"
#include "DHTesp.h"
#include <ESP8266WiFi.h>
#include "Adafruit_MQTT.h"
#include "Adafruit_MQTT_Client.h"

/************************* WiFi Access Point *********************************/
#define WLAN_SSID       "KOFK.D"
#define WLAN_PASS       "3c90660d9ebc"

/************************* Adafruit.io Setup *********************************/
#define AIO_SERVER      "io.adafruit.com"
#define AIO_SERVERPORT  1883                   // use 8883 for SSL
#define AIO_USERNAME    "mercyfulgpf"
#define AIO_KEY         "634858eedae744a086f16e4763a06b53"

/****** GLOBALS ******/
WiFiClient client;
Adafruit_MQTT_Client mqtt(&client, AIO_SERVER, AIO_SERVERPORT, AIO_USERNAME, AIO_KEY);
Adafruit_MQTT_Publish temperature = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/historical-temperature.temp");

Adafruit_7segment matrix = Adafruit_7segment();
DHTesp dht;

int cycleCounter;
void MQTT_connect();

void setup() 
{
  //Init variables
  cycleCounter = 0;
  //Init serial
  Serial.begin(115200);
  Serial.println();

  Serial.print("Initializing DHT... ");
  //Init DHT
  dht.setup(14, DHTesp::DHT11);
  Serial.println("Done");

  Serial.print("Initializing display... ");
  //Init 7Seg display
  matrix.begin(0x70);
  delay(200);
  matrix.println(8888);
  matrix.writeDisplay();
  delay(1000);
  matrix.println(0);
  matrix.writeDisplay();
  Serial.println("Done");

  Serial.print("Connecting to WLAN [");
  Serial.print(WLAN_SSID);
  Serial.print("].");
  
  WiFi.begin(WLAN_SSID, WLAN_PASS);
  while (WiFi.status() != WL_CONNECTED) {
    delay(250);
    Serial.print(".");
  }
  Serial.println(" Done");

  Serial.print("WiFI connected [");
  Serial.print(WiFi.localIP());
  Serial.println("]");

  Serial.print("Initializing MQTT subscriptions...");
  //mqtt.subscribe(&onoffbutton);
  Serial.println("Done");
  
  //
  Serial.println("Initialization complete!");
}

void MQTT_connect() {
  int8_t ret;

  // Stop if already connected.
  if (mqtt.connected()) {
    return;
  }

  Serial.print("Connecting to MQTT... ");

  uint8_t retries = 3;
  while ((ret = mqtt.connect()) != 0) 
  { // connect will return 0 for connected
       Serial.println(mqtt.connectErrorString(ret));
       Serial.println("Retrying MQTT connection in 5 seconds...");
       mqtt.disconnect();
       delay(5000);  // wait 5 seconds
       retries--;
       if (retries == 0) 
       {
         // basically die and wait for WDT to reset me
         while (1);
       }
  }
  Serial.println("MQTT Connected!");
}

void sendReadings(int temp, int humidity)
{
  Serial.println("Sending temperature readings...");
  MQTT_connect();
  if (! temperature.publish(temp)) 
  {
    Serial.println(F("Sending data failed"));
  } 
  else 
  {
    Serial.println(F("Sending data success"));
  }

  if(! mqtt.ping()) 
  {
    mqtt.disconnect();
  }
}

void loop() 
{
  if (cycleCounter == 1000 || cycleCounter == 0)
  {
    float humidity = dht.getHumidity();
    float temperature = dht.getTemperature();
    
    matrix.println(8888);
    matrix.drawColon(false);
    matrix.writeDisplay();
    cycleCounter = 0;
    Serial.print("Temp Measurement: ");
    Serial.println(temperature);
    sendReadings(temperature, -1);
    
    matrix.println(int(temperature));
    matrix.writeDisplay();
  }
  else if (cycleCounter % 5 == 0)
  {
    //Serial.print("Cycle check at ");
    //Serial.println(cycleCounter);
    matrix.drawColon(cycleCounter % 10 == 0);
    matrix.writeDisplay();
  }

  cycleCounter++;
  delay(100);
}
