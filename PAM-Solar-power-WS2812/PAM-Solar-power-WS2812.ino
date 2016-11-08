////////////////////////////////////////////////////////////////////////////////////
//
//                               PI and more
//                  ESP8266, solar power reading and displaying
//
// https://piandmore.wordpress.com/2016/11/08/fun-with-esp-ws2812-and-homewizard/
//
////////////////////////////////////////////////////////////////////////////////////
//
// The library to address the WS2812 LEDs
//
#include <Adafruit_NeoPixel.h>
//
// The library to grant wifi usage to the ESP8266
//
#include <ESP8266WiFi.h>
//
// My library with some standard wifi functions.
// This can be found on GitHub
// 
#include <PAM_WiFiConnect.h>
#include <PAM_WiFiClient.h>

#define CIRCLEPIN 2
#define PIXELCOUNT 12
#define POWERPERLED1 250
#define POWERPERLED2 125
#define POWERPERLED3 50

Adafruit_NeoPixel pixels = Adafruit_NeoPixel(PIXELCOUNT,CIRCLEPIN,
  NEO_GRB+NEO_KHZ800);

int solarPower = 0;
int powerConsumption = 0;
float solarPowerDay = 0;
float powerConsumptionDay = 0;

void setup() {
  Serial.begin(115200);
  wifiConnect("YourSSID","YourPassword");
  pixels.begin();
  for(byte i=0;i<PIXELCOUNT;i++){
    pixels.setPixelColor(i, pixels.Color(0,0,0));
  }
  pixels.show();
}

void readSolar () {
  String t = getURL("YourHomewizardUrl","/YourHWPassword/get-status",80);
  t = t.substring(t.indexOf("energylinks"));
  t = t.substring(t.indexOf("s1"));
  t = t.substring(t.indexOf("po")+4);
  int xsolarPower = t.substring(0,t.indexOf(",")).toInt();
  t = t.substring(t.indexOf("dayTotal")+10);
  int xsolarPowerDay = t.substring(0,t.indexOf(",")).toFloat()*1000;
  t = t.substring(t.indexOf("used"));
  t = t.substring(t.indexOf("po")+4);
  int xpowerConsumption = t.substring(0,t.indexOf(",")).toInt();
  t = t.substring(t.indexOf("dayTotal")+10);
  int xpowerConsumptionDay = t.substring(0,t.indexOf(",")).toFloat()*1000;
  t = t.substring(0,t.indexOf("heatlinks"));
  Serial.print("Solar       now :");
  Serial.println(solarPower);
  Serial.print("Consumption now :");
  Serial.println(powerConsumption);
  Serial.print("Solar       day :");
  Serial.println(solarPowerDay);
  Serial.print("Consumption day :");
  Serial.println(powerConsumptionDay);
  Serial.println();
  if (xsolarPower!=0 || xpowerConsumption!=0) {
    solarPower = xsolarPower;
    solarPowerDay = xsolarPowerDay;
    powerConsumption = xpowerConsumption;
    powerConsumptionDay = xpowerConsumptionDay;
  }
}

void loop() {
  readSolar();

  byte solarLed;
  byte powerLed;
  byte base;
  if (_max(solarPower,powerConsumption)<=POWERPERLED3*(PIXELCOUNT-2)) {
    solarLed = round((solarPower+POWERPERLED3/2)/POWERPERLED3)+4;
    powerLed = round((powerConsumption+POWERPERLED3/2)/POWERPERLED3)+4;
    base = 2;
    for (byte i=0;i<base;i++) {
      pixels.setPixelColor(i,pixels.Color(0,42,0));
    }
  } else if (_max(solarPower,powerConsumption)<=POWERPERLED2*(PIXELCOUNT-1)) {
    solarLed = round((solarPower+POWERPERLED2/2)/POWERPERLED2)+2;
    powerLed = round((powerConsumption+POWERPERLED2/2)/POWERPERLED2)+2;
    base = 1;
    pixels.setPixelColor(0,pixels.Color(0,42,0));
  } else {
    solarLed = round((solarPower+POWERPERLED1/2)/POWERPERLED1);
    powerLed = round((powerConsumption+POWERPERLED1/2)/POWERPERLED1);
    base = 0;
    if (solarLed>PIXELCOUNT || powerLed>PIXELCOUNT) {
      base = _max(solarLed,powerLed)-PIXELCOUNT;
      for (byte i=0;i<base;i++) {
        pixels.setPixelColor(i,pixels.Color(0,0,42));
      }
    }
  }
  for (byte i=base;i<_min(solarLed,powerLed)-base;i++) {
    pixels.setPixelColor(i,pixels.Color(110,27,0));
  }
  if (powerLed>solarLed) {
    for (byte i=_max(base,_min(solarLed,powerLed)-base);i<powerLed-base;i++) {
      pixels.setPixelColor(i,pixels.Color(55,0,0));
    }
    for (byte i=powerLed-base;i<PIXELCOUNT;i++) {
      pixels.setPixelColor(i,pixels.Color(0,0,0));
    }
  } else {
    for (byte i=_max(base,_min(solarLed,powerLed)-base);i<solarLed-base;i++) {
      pixels.setPixelColor(i,pixels.Color(55,42,0));
    }
    for (byte i=solarLed-base;i<PIXELCOUNT;i++) {
      pixels.setPixelColor(i,pixels.Color(0,0,0));
    }
  }
  pixels.show();
  delay(500);
}

