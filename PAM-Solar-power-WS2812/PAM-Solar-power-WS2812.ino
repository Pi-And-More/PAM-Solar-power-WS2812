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
// My libraries with some standard wifi functions.
// These can be found on GitHub
// More information can be found on Pi And More:
// https://piandmore.wordpress.com/tag/pam_wificonnect/
// https://piandmore.wordpress.com/tag/pam_wificlient
// 
#include <PAM_WiFiConnect.h>
#include <PAM_WiFiClient.h>

//
// The WS2812 circle is connected on pin 2
//
#define CIRCLEPIN 2
//
// There are 12 pixels in the circle
//
#define PIXELCOUNT 12
//
// Scale level 1 is 250W per led
// Scale level 2 is 125W per led
// Scale level 3 is 50W per led
//
#define POWERPERLED1 250
#define POWERPERLED2 125
#define POWERPERLED3 50

//
// Initialize the neopixel library
//
Adafruit_NeoPixel pixels = Adafruit_NeoPixel(PIXELCOUNT,CIRCLEPIN,
  NEO_GRB+NEO_KHZ800);

//
// The variables to store the solar power produced (current and day total)
// and the power consumped (current and day total)
//
int solarPower = 0;
int powerConsumption = 0;
float solarPowerDay = 0;
float powerConsumptionDay = 0;

void setup() {
  //
  // Set up Serial
  //
  Serial.begin(115200);
  //
  // Connect to wifi. Remember to use your own credentials
  //
  wifiConnect("YourSSID","YourPassword");
  //
  // Start the neopixel instance and turn all leds black
  //
  pixels.begin();
  for(byte i=0;i<PIXELCOUNT;i++){
    pixels.setPixelColor(i, pixels.Color(0,0,0));
  }
  pixels.show();
}

//
// Function to retrieve the current status from the homewizard
// which is at http://your.home.wizard.url.or.ip/your.hw.password/get-status
// This returns a json file with all current lights, sensors etc and includes
// the power readings
//
void readSolar () {
  //
  // Retrieve the current status
  //
  String t = getURL("YourHomewizardUrl","/YourHWPassword/get-status",80);
  //
  // Walk through the json file and parse to get the required information
  //
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
  //
  // If the power readings are not 0 we fill the global variables
  //
  if (xsolarPower!=0 || xpowerConsumption!=0) {
    solarPower = xsolarPower;
    solarPowerDay = xsolarPowerDay;
    powerConsumption = xpowerConsumption;
    powerConsumptionDay = xpowerConsumptionDay;
  }
}

//
// The main loop function
//
void loop() {
  //
  // Get the current readings
  //
  readSolar();

  //
  // Variables to store the number of solar leds, power leds and where to start (base)
  // The base is used in case a scale other than scale 1 is used.
  //
  byte solarLed;
  byte powerLed;
  byte base;
  //
  // Determine which scale to use and determine in that scale how many leds are to be
  // shown for power consumption and solar production.
  //
  // If the maximum value of solar power and power consumption can be shown on scale 3
  // (with 2 leds deducted to show the lower scale), use this scale
  //
  if (_max(solarPower,powerConsumption)<=POWERPERLED3*(PIXELCOUNT-2)) {
    solarLed = round((solarPower+POWERPERLED3/2)/POWERPERLED3)+4;
    powerLed = round((powerConsumption+POWERPERLED3/2)/POWERPERLED3)+4;
    base = 2;
    for (byte i=0;i<base;i++) {
      pixels.setPixelColor(i,pixels.Color(0,42,0));
    }
  //
  // If the maximum value of solar power and power consumption can be shown on scale 2
  // (with 1 led deducted to show the lower scale), use this scale
  //
  } else if (_max(solarPower,powerConsumption)<=POWERPERLED2*(PIXELCOUNT-1)) {
    solarLed = round((solarPower+POWERPERLED2/2)/POWERPERLED2)+2;
    powerLed = round((powerConsumption+POWERPERLED2/2)/POWERPERLED2)+2;
    base = 1;
    pixels.setPixelColor(0,pixels.Color(0,42,0));
  //
  // Use the standard scale
  //
  } else {
    solarLed = round((solarPower+POWERPERLED1/2)/POWERPERLED1);
    powerLed = round((powerConsumption+POWERPERLED1/2)/POWERPERLED1);
    base = 0;
    //
    // Check whether the maximum of solar production and power consumption
    // requires to light up blue leds (which mean twice the value of a
    // scale 1 led
    //
    if (solarLed>PIXELCOUNT || powerLed>PIXELCOUNT) {
      base = _max(solarLed,powerLed)-PIXELCOUNT;
      for (byte i=0;i<base;i++) {
        pixels.setPixelColor(i,pixels.Color(0,0,42));
      }
    }
  }
  //
  // Show the orange leds which mean both power consumption and solar production
  //
  for (byte i=base;i<_min(solarLed,powerLed)-base;i++) {
    pixels.setPixelColor(i,pixels.Color(110,27,0));
  }
  //
  // If more power is consumed than produced, turn the required leds red
  // else turn the required leds yellow
  //
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

