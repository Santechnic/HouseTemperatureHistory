#include "DHTesp.h"
#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include "Adafruit_MQTT.h"
#include "Adafruit_MQTT_Client.h"
#include <Adafruit_SSD1306.h>

/************************* Display Params     *********************************/
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
#define OLED_RESET     12 // Reset pin # (or -1 if sharing Arduino reset pin)

/************************* WiFi Access Point *********************************/
#define WLAN_SSID       "KOFK2"
#define WLAN_PASS       "deev4n762x39deev4n762x39"

/************************* Adafruit.io Setup *********************************/
#define AIO_SERVER      "io.adafruit.com"
#define AIO_SERVERPORT  1883                   // use 8883 for SSL
#define AIO_USERNAME    "mercyfulgpf"
#define AIO_KEY         "634858eedae744a086f16e4763a06b53"

/****** GLOBALS ******/
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
WiFiClient client;
ESP8266WiFiMulti wifiMulti;
Adafruit_MQTT_Client mqtt(&client, AIO_SERVER, AIO_SERVERPORT, AIO_USERNAME, AIO_KEY);
Adafruit_MQTT_Publish feedTemp = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/historical-temperature.temp");
Adafruit_MQTT_Publish feedHumid = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/historical-temperature.humidity");



DHTesp dht;

int cycleCounter;
void MQTT_connect();
float minTemp;
float maxTemp;
float lastReadingHumidity;
float lastReadingTemp;

void setup() 
{
  //Init variables
  cycleCounter = 0;
  minTemp = 99;
  maxTemp = -99;
  
  //Init serial
  Serial.begin(115200);
  Serial.println();

  Serial.print("Initializing DHT... ");
  //Init DHT
  dht.setup(14, DHTesp::DHT22);
  Serial.println("Done");

  Serial.print("Initializing display... ");
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3D)) 
  { // Address 0x3D for 128x64
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
  }
  Serial.println("Done");

  wifiMulti.addAP("GG45722", "aadef32146");
  wifiMulti.addAP("KOFK.D", "3c90660d9ebc");
  wifiMulti.addAP("KOFK2", "deev4n762x39deev4n762x39");

  while (wifiMulti.run() != WL_CONNECTED) 
  {
    delay(250);
    Serial.print(".");
  }

  Serial.print("WiFI connected [");
  Serial.print(WiFi.localIP());
  Serial.print("] to ");
  Serial.println(WiFi.SSID());

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

void printStatusScreen(int temp, int humidity, int minTemp, int maxTemp)
{
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(2); // Draw 2X-scale text
  display.setCursor(5, 0);
  display.print(F("TEMPLOGGER"));
  display.setTextSize(1); // Draw 2X-scale text
  display.setCursor(0, 15);
  display.print(F("temperature="));
  display.print(temp);
  display.setCursor(0, 25);
  display.print(F("humidity="));
  display.print(humidity);
  display.drawRoundRect(0, 35, 128, 28, 4, SSD1306_WHITE);
  display.setCursor(20, 40);
  display.print(F("min="));
  display.print(minTemp);
  display.print(F("  max="));
  display.print(maxTemp);
  //display.setCursor(32, 50);
  // display.print(F("counter="));
  //display.print(cycleCounter);
  display.drawRect(10, 50, 108, 10, SSD1306_WHITE);
  display.display();
}

void drawProgressBar(int val)
{
  int progress = int((float(val) * (float(108.0 - 10.0) / 100.0)));
  Serial.print("Progress: ");
  Serial.println(progress);
  display.fillRect(10, 50, progress, 10, SSD1306_WHITE);
  display.display();
}

void printSendingScreen()
{
  display.clearDisplay();
  display.setTextSize(2); // Draw 2X-scale text
  display.setCursor(20, 10);
  display.print(F("Sending"));
  display.setCursor(38, 34);
  display.print(F("data"));
  display.display();
}

void sendReadings(float temp, float humidity)
{
  Serial.println("Sending temperature readings...");
  MQTT_connect();
  if (! feedTemp.publish(temp)) 
  {
    Serial.println(F("Sending temperature data failed"));
  } 
  else 
  {
    Serial.println(F("Sending temperature data success"));
  }
   if (! feedHumid.publish(humidity)) 
  {
    Serial.println(F("Sending humidity data failed"));
  } 
  else 
  {
    Serial.println(F("Sending humidity data success"));
  }

  if(! mqtt.ping()) 
  {
    mqtt.disconnect();
  }
}

void loop() 
{
  if (cycleCounter >= 240 || cycleCounter == 0)
  {
    //printSendingScreen();
    lastReadingHumidity = dht.getHumidity();
    lastReadingTemp = dht.getTemperature();

    if (lastReadingTemp > maxTemp)
      maxTemp = lastReadingTemp;
    if (lastReadingTemp < minTemp)
      minTemp = lastReadingTemp;

    cycleCounter = 0;
    Serial.print("Temp Measurement: ");
    Serial.println(lastReadingTemp);
    sendReadings(lastReadingTemp, lastReadingHumidity);
    Serial.print("Maximum temperature read: ");
    Serial.print(maxTemp);
    Serial.print("C, Minimum temperature recorded: ");
    Serial.print(minTemp);
    Serial.println("C");
    //printStatusScreen(lastReadingTemp, lastReadingHumidity, minTemp, maxTemp);
  }
//  else if (cycleCounter % 5 == 0)
//  {
//    //printStatusScreen(lastReadingTemp, lastReadingHumidity, minTemp, maxTemp);
//    drawProgressBar(cycleCounter / 10);
//  }

  cycleCounter++;
  delay(1000);
}
