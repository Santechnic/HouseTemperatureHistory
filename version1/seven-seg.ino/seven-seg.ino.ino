#include "Adafruit_LEDBackpack.h"
#include "DHTesp.h"


Adafruit_7segment matrix = Adafruit_7segment();
DHTesp dht;

int c;

void setup() 
{
  //Init variables
  c = 0;
  //Init serial
  Serial.begin(115200);
  Serial.println();

  //Init DHT
  dht.setup(14, DHTesp::DHT11);
   
  //Init 7Seg display
  matrix.begin(0x70);
  delay(200);
  matrix.println(8888);
  matrix.writeDisplay();
  delay(1000);
  matrix.println(0);
  matrix.writeDisplay();
  
  //
  Serial.println("Done initializing");
}

void loop() 
{
  if (c == 1000 || c == 0)
  {
    float humidity = dht.getHumidity();
    float temperature = dht.getTemperature();
    
    matrix.println(int(temperature));
    matrix.drawColon(false);
    matrix.writeDisplay();
    c = 0;
    Serial.print("Temp Measurement: ");
    Serial.println(temperature);
  }
  else if (c % 5 == 0)
  {
    Serial.print("Colon check at ");
    Serial.println(c);
    matrix.drawColon(c % 10 == 0);
    matrix.writeDisplay();
  }

  c++;
  delay(100);
}
