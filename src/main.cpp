#include <Arduino.h>
#include <FastLED.h>
#include <RTClib.h>
#include <OneButton.h>

#define STRIP_DATA_PIN 12
#define COLOR_ORDER GRB
#define BRIGHTNESS_DAY 45 // 0-255
#define BRIGHTNESS_NIGHT 5
#define COLS 11
#define ROWS 10
#define NUM_LEDS (COLS * ROWS)
#define BTN_PIN 3
// 4 corner LEDs wired to a ~300 ohm resistor
#define LED_1 4
#define LED_2 5
#define LED_3 6
#define LED_4 7

// use: https://fastled.io/docs/df/da2/group__lib8tion.html

// WS2812B strip
CRGB leds[NUM_LEDS];

// control button
OneButton button;

// RTC_DS3231 rtc;  // TODO: use when RTC component is here
RTC_Millis rtc; // use for now

struct Pattern
{
  uint8_t start;
  uint8_t length;
  CRGB color;

  // need constructor for constexpr
  constexpr Pattern() : start(0), length(0), color(CRGB::Black) {}
  constexpr Pattern(uint8_t s, uint8_t l, CRGB c) : start(s), length(l), color(c) {}
};

enum Error
{
  RTC_ERROR
};

// forward declarations
void setGridTime(uint8_t, uint8_t);
void setGridError(Error);
bool isNight(DateTime);
void setRTCtime(uint8_t, uint8_t);
void setText(Pattern);
void setMinuteHand(uint8_t);

void buttonSingleClick()
{
  Serial.println("Button - Single click");
  // TODO: increase by a minute
}
void buttonDoubleClick()
{
  Serial.println("Button - Double click");
  // TODO: increase by an hour
}
void buttonLongPress()
{
  Serial.println("Button - Long press");
  // TODO: seek through: 5m per tick
}

// led index (row, column)
constexpr uint8_t ledIndex(uint8_t r, uint8_t c)
{
  return (r & 1) ? (r * COLS + (COLS - 1 - c)) : (r * COLS + c);
}

constexpr Pattern fromLine(uint8_t start_row, uint8_t start_column, uint8_t size, CRGB color)
{
  // uint8_t start_index = (start_row & 1) ? (start_row * COLS + (COLS - 1 - start_column)) : (start_row * COLS + start_column);
  // return Pattern{start_index, size, color};
  return (start_row & 1)
             ? Pattern{ledIndex(start_row, start_column), size, color}         // forward (even row)
             : Pattern{ledIndex(start_row, start_column) - size, size, color}; // backward (odd row)
}

// intro text
const Pattern TEXT_ES = fromLine(0, 0, 2, CRGB::White);
const Pattern TEXT_IST = fromLine(0, 3, 3, CRGB::White);
const Pattern TEXT_FUENF = fromLine(0, 7, 4, CRGB::White);
const Pattern TEXT_ZEHN = fromLine(1, 0, 4, CRGB::White);
const Pattern TEXT_ZWANZIG = fromLine(1, 4, 7, CRGB::White);
const Pattern TEXT_DREI = fromLine(2, 0, 4, CRGB::White);
const Pattern TEXT_VIERTEL = fromLine(2, 4, 7, CRGB::White);
const Pattern TEXT_VOR = fromLine(3, 0, 3, CRGB::White);
const Pattern TEXT_NACH = fromLine(3, 7, 4, CRGB::White);
const Pattern TEXT_HALB = fromLine(4, 0, 4, CRGB::White);
const Pattern TEXT_UHR = fromLine(9, 8, 3, CRGB::White);
// hours
const Pattern TEXT_1 = fromLine(5, 0, 4, CRGB::White);
const Pattern TEXT_2 = fromLine(5, 7, 4, CRGB::White);
const Pattern TEXT_3 = fromLine(6, 0, 4, CRGB::White);
const Pattern TEXT_4 = fromLine(6, 7, 4, CRGB::White);
const Pattern TEXT_5 = fromLine(4, 7, 4, CRGB::White);
const Pattern TEXT_6 = fromLine(7, 0, 5, CRGB::White);
const Pattern TEXT_7 = fromLine(8, 0, 6, CRGB::White);
const Pattern TEXT_8 = fromLine(7, 7, 4, CRGB::White);
const Pattern TEXT_9 = fromLine(9, 3, 4, CRGB::White);
const Pattern TEXT_10 = fromLine(9, 0, 4, CRGB::White);
const Pattern TEXT_11 = fromLine(4, 5, 3, CRGB::White);
const Pattern TEXT_12 = fromLine(8, 6, 5, CRGB::White);
// errors
const Pattern TEXT_ERROR = fromLine(3, 3, 3, CRGB::Red);
const Pattern TEXT_ERROR_1 = fromLine(4, 1, 1, CRGB::Red); // a letter A
const Pattern TEXT_ERROR_2 = fromLine(4, 3, 1, CRGB::Red); // a letter B
const Pattern TEXT_ERROR_3 = fromLine(7, 2, 1, CRGB::Red); // a letter C

void setup()
{
  Serial.begin(9600);
  Serial.println("Starting up...");

  // set up LED strip
  FastLED.addLeds<WS2812B, STRIP_DATA_PIN, COLOR_ORDER>(leds, NUM_LEDS);
  FastLED.setMaxPowerInMilliWatts(3500);
  // show_at_max_brightness_for_power(); // automatically determines brightness (based on power setting)

  // set up LEDs
  pinMode(LED_1, OUTPUT);
  pinMode(LED_2, OUTPUT);
  pinMode(LED_3, OUTPUT);
  pinMode(LED_4, OUTPUT);

  rtc.begin(DateTime(2025, 1, 1, 0, 0, 0));
  // check if there was a power loss since last rtc use
  // if (rtc.lostPower())
  // {
  //   Serial.println("RTC lost power, let's set the time!");
  // }

  // setup button
  button.setup(
      BTN_PIN,      // Input pin for the button
      INPUT_PULLUP, // INPUT and enable the internal pull-up resistor
      true          // Button is active LOW
  );
  button.attachClick(buttonSingleClick);
  button.attachDoubleClick(buttonDoubleClick);
  button.attachDuringLongPress(buttonLongPress);
  button.setLongPressIntervalMs(200); // frequency to fire long press function if long press is detected

  Serial.println("Setup Complete");

  // "startup animation"
  FastLED.setBrightness(BRIGHTNESS_DAY);
  fill_solid(leds, NUM_LEDS, CRGB::Blue);
  FastLED.show();
  delay(5000);
}

void loop()
{
  button.tick();

  DateTime now = rtc.now();

  if (isNight(now))
  {
    FastLED.setBrightness(BRIGHTNESS_NIGHT);
  }
  else
  {
    FastLED.setBrightness(BRIGHTNESS_DAY);
  }

  // update time
  if (now.second() == 0)
  {
    setGridTime(now.hour(), now.minute());
  }

  // call multiple times a second to make sure we dont miss the minute increase
  delay(50);
}

bool isNight(DateTime time)
{
  if (time.hour() < 7 || time.hour() > 21)
  {
    return true;
  }
  return false;
}

void setRTCtime(uint8_t hour, uint8_t minute)
{
  // date and seconds are not relevant
  rtc.adjust(DateTime(2025, 1, 1, hour, minute, 0));
}

void setGridTime(uint8_t hour, uint8_t minute)
{
  Serial.print("Setting time: ");
  Serial.print(hour);
  Serial.print(":");
  Serial.println(minute);

  // reset
  fill_solid(leds, NUM_LEDS, CRGB::Black);

  uint8_t rest = minute % 5;
  setMinuteHand(rest);
  // modify minute for easy handling
  minute = minute - rest;

  /*
  10:00 = ES IST ZEHN UHR
  10:05 = ES IST FÜNF NACH ZEHN
  10:10 = ES IST ZEHN NACH ZEHN
  10:15 = ES IST VIERTEL ELF
  10:20 = ES IST ZWANZIG NACH ZEHN
  10:25 = ES IST FÜNF VOR HALB ELF
  10:30 = ES IST HALB ELF
  10:35 = ES IST FÜNF NACH HALB ELF
  10:40 = ES IST ZWANZIG	VOR ELF
  10:45 = ES IST DREIVIERTEL ELF
  10:50 = ES IST ZEHN VOR ELF
  10:55 = ES IST FÜNF VOR ELF
  */
  Pattern currHour, nextHour;

  switch (hour)
  {
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

  switch (minute)
  {
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

void setText(Pattern pattern)
{
  for (uint8_t i = pattern.start; i < pattern.length; i++)
  {
    leds[i] = pattern.color;
  }
}

// minute between 0 and 4
void setMinuteHand(uint8_t leds)
{
  switch (leds)
  {
  case 1:
    digitalWrite(LED_1, HIGH);
    digitalWrite(LED_2, LOW);
    digitalWrite(LED_3, LOW);
    digitalWrite(LED_4, LOW);
    break;
  case 2:
    digitalWrite(LED_1, LOW);
    digitalWrite(LED_2, HIGH);
    digitalWrite(LED_3, LOW);
    digitalWrite(LED_4, LOW);
    break;
  case 3:
    digitalWrite(LED_1, LOW);
    digitalWrite(LED_2, LOW);
    digitalWrite(LED_3, HIGH);
    digitalWrite(LED_4, LOW);
    break;
  case 4:
    digitalWrite(LED_1, LOW);
    digitalWrite(LED_2, LOW);
    digitalWrite(LED_3, LOW);
    digitalWrite(LED_4, HIGH);
    break;
  default:
    digitalWrite(LED_1, LOW);
    digitalWrite(LED_2, LOW);
    digitalWrite(LED_3, LOW);
    digitalWrite(LED_4, LOW);
    break;
  }
}

void setGridError(Error err)
{
  Serial.print("Setting error: ");
  Serial.println(err);

  // clear
  fill_solid(leds, NUM_LEDS, CRGB::Black);

  // set
  setText(TEXT_ERROR);
  switch (err)
  {
  case RTC_ERROR:
    setText(TEXT_ERROR_1);
    break;
  }

  // update
  FastLED.show();
}
