#include <FastLED.h>

#define NUM_LEDS (11*10+4)
#define PIN 12  // Data pin connected to D4
#define COLOR_ORDER GRB

CRGB leds[NUM_LEDS];

void setup() {
  FastLED.addLeds<WS2812B, PIN, COLOR_ORDER>(leds, NUM_LEDS);
  FastLED.setMaxPowerInMilliWatts(3500);
  // show_at_max_brightness_for_power(); // automatically determines brightness (based on power setting)

  FastLED.setBrightness(50); // 0-255?
  fill_solid(leds, NUM_LEDS, CRGB::Green);
  FastLED.show();
}

void loop() {
}
