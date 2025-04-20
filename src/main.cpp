#include <Arduino.h>

#define PIN 4  // Data pin connected to D4
#define LED_TYPE WS2812B
#define COLOR_ORDER GRB
#define BRIGHTNESS_DAY 45  // 0-255
#define BRIGHTNESS_NIGHT 5
#define COLS 11
#define ROWS 10
// FIXME: #define NUM_LEDS (COLS * ROWS)
#define NUM_LEDS 12


// use: https://fastled.io/docs/df/da2/group__lib8tion.html

CRGB leds[NUM_LEDS];

// RTC_DS32131 rtc;  // TODO: use when component is here
RTC_Millis rtc;  // use for now

// forward declarations
void setGridTime(uint8_t, uint8_t);
bool isNight();
void setRTCtime(uint8_t, uint8_t);
void setGridError(uint8_t);
void setText(uint8_t, uint8_t);
void setMinuteHand(uint8_t);

// TODO: would be safer to just save start index and length!
TEXT_ES;
TEXT_IST;
TEXT_FUENF;
TEXT_ZEHN;
TEXT_ZWANZIG;
TEXT_DREI;
TEXT_VIERTEL;
TEXT_VOR;
TEXT_NACH;
TEXT_HALB;
TEXT_1;
TEXT_2;
TEXT_3;
TEXT_4;
TEXT_5;
TEXT_6;
TEXT_7;
TEXT_8;
TEXT_9;
TEXT_10;
TEXT_11;
TEXT_12;
TEXT_UHR;



void setup() {
  FastLED.addLeds<LED_TYPE, PIN, COLOR_ORDER>(leds, NUM_LEDS);
  FastLED.setMaxPowerInMilliWatts(3500);
  // show_at_max_brightness_for_power(); // automatically determines brightness (based on power setting)

  if (!rtc.begin()) {
    Serial.println("Couldn't find RTC");
    Serial.flush();
    setGridError();
  }

  // check if there was a power loss since last rtc use
  if (rtc.lostPower()) {
    Serial.println("RTC lost power, let's set the time!");
  }

  if (isNight()) {
    FastLED.setBrightness(BRIGHTNESS_NIGHT);
  } else {
    FastLED.setBrightness(BRIGHTNESS_DAY);
  }
}

void loop() {
  // TODO: check for button press

  // Simple animation: Red color sweep
  // for (int i = 0; i < NUM_LEDS; i++) {
  //   leds[i] = CRGB::White;  // Red color
  //   FastLED.show();
  //   delay(50);
  //   leds[i] = CRGB::Black;  // Turn off the LED (in the next iteration)
  // }

  DateTime now = rtc.now();

  if (now.second() == 0) {
    setGridTime(now.hour(), now.minute())
  }

  // call 10x a second to make sure we dont miss the minute increase
  delay(100);
}



bool isNight() {
  DateTime now = rtc.now();
  if (now.hours() < 7 || now.hours() > 21) {
    return true;
  } else {
    return false;
  }
}


void setRTCtime(uint8_t hour, uint8_t minute) {
  // date and seconds are not relevant
  rtc.adjust(DateTime(2025, 1, 1, hour, minute, 0));
}



void setGridTime(uint8_t hour, uint8_t minute) {
  // reset
  fill_solid(leds, NUM_LEDS, CRGB::Black);

  setMinuteHand(minute);

  // modify minute for easy handling
  minute = minute - (minute % 5);

  /*
  10:00 = ES IST ZEHN UHR
  10:05 = ES IST FÜNF NACH ZEHN
  10:10 = ES IST ZEHN NACH ZEHN
  10:15 = ES IST VIERTEL	ELF
  10:20 = ES IST ZWANZIG	NACH ZEHN
  10:25 = ES IST FÜNF VOR HALB ELF
  10:30 = ES IST HALB ELF
  10:35 = ES IST FÜNF NACH HALB ELF
  10:40 = ES IST ZWANZIG	VOR ELF 
  10:45 = ES IST DREIVIERTEL ELF
  10:50 = ES IST ZEHN VOR ELF
  10:55 = ES IST FÜNF VOR ELF
  */
  Pattern currHour;
  Pattern nextHour;

  switch (hour) {
    case 0:
    case 12:
      currHour = TEXT_12;
      nextHour = TEXT_1;
      break;

    case 1:
    case 13:
      currHour = TEXT_1;
      nextHour = TEXT_2;
      break;

    case 2:
    case 14:
      currHour = TEXT_2;
      nextHour = TEXT_3;
      break;

    case 3:
    case 15:
      currHour = TEXT_3;
      nextHour = TEXT_4;
      break;

    case 4:
    case 16:
      currHour = TEXT_4;
      nextHour = TEXT_5;
      break;

    case 5:
    case 17:
      currHour = TEXT_5;
      nextHour = TEXT_6;
      break;

    case 6:
    case 18:
      currHour = TEXT_6;
      nextHour = TEXT_7;
      break;

    case 7:
    case 19:
      currHour = TEXT_7;
      nextHour = TEXT_8;
      break;

    case 8:
    case 20:
      currHour = TEXT_8;
      nextHour = TEXT_9;
      break;

    case 9:
    case 21:
      currHour = TEXT_9;
      nextHour = TEXT_10;
      break;

    case 10:
    case 22:
      currHour = TEXT_10;
      nextHour = TEXT_11;
      break;

    case 11:
    case 23:
      currHour = TEXT_11;
      nextHour = TEXT_12;
      break;
  }

  setText(TEXT_ES);
  setText(TEXT_IST);

  switch (minute) {
    case 0:
      setText(currHour);
      setText(TEXT_UHR);
      break;
    case 5:
      setText(TEXT_FUENF);
      setText(TEXT_NACH);
      setText(currHour);
      break;
    case 10:
      setText(TEXT_ZEHN);
      setText(TEXT_NACH);
      setText(currHour);
      break;
    case 15:
      setText(TEXT_VIERTEL);
      setText(nextHour);
      break;
    case 20:
      setText(TEXT_ZWANZIG);
      setText(TEXT_NACH);
      setText(currHour);
      break;
    case 25:
      setText(TEXT_FUENF);
      setText(TEXT_VOR);
      setText(TEXT_HALB);
      setText(nextHour);
      break;
    case 30:
      setText(TEXT_HALB);
      setText(nextHour);
      break;
    case 35:
      setText(TEXT_FUENF);
      setText(TEXT_NACH);
      setText(TEXT_HALB);
      setText(nextHour);
      break;
    case 40:
      setText(TEXT_ZWANZIG);
      setText(TEXT_VOR);
      setText(nextHour);
      break;
    case 45:
      setText(TEXT_DREI);
      setText(TEXT_VIERTEL);
      setText(nextHour);
      break;
    case 50:
      setText(TEXT_ZEHN);
      setText(TEXT_VOR);
      setText(nextHour);
      break;
    case 55:
      setText(TEXT_FUENF);
      setText(TEXT_VOR);
      setText(nextHour);
  }

  // update leds
  FastLED.show();
}

void setText(uint8_t start, uint8_t length) {
  // FIXME: this needs to also respect the serpentine pattern!!!
  for (uint8_t i = start; i < length; i++) {
    leds[i] = CRGB::White;
  }
}