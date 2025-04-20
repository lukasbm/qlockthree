#include <FastLED.h>

#define NUM_LEDS 12
#define PIN 4  // Data pin connected to D4
#define COLOR_ORDER GRB

CRGB leds[NUM_LEDS];

void setup() {
  FastLED.addLeds<WS2812B, PIN, COLOR_ORDER>(leds, NUM_LEDS);
  FastLED.setMaxPowerInMilliWatts(3500);
  // show_at_max_brightness_for_power(); // automatically determines brightness (based on power setting)

  // check if there was a power loss since last rtc use
  // if (rtc.lostPower())
  // {
  //   Serial.println("RTC lost power, let's set the time!");
  // }

  FastLED.setBrightness(50);
  fill_solid(leds, NUM_LEDS, CRGB::Green);
  delay(3000);
  FastLED.show();
}

void loop() {
}
