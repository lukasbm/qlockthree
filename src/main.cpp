#include <Arduino.h>
#include <FastLED.h>
#include <RTClib.h>
#include <OneButton.h>
#include <Wire.h>

#define STRIP_DATA_PIN 12 // D12
#define LDR_PIN A2        // A2
#define COLOR_ORDER GRB
#define BRIGHTNESS_MAX 255
#define BRIGHTNESS_MIN 1
#define COLS 11
#define ROWS 10
#define NUM_LEDS (COLS * ROWS + 4)
#define BTN_PIN 11 // D11

// use: https://fastled.io/docs/df/da2/group__lib8tion.html

// WS2812B strip
CRGB leds[NUM_LEDS];

// control button
OneButton button;

RTC_DS3231 rtc; // uses i2c (sda = A4, scl = A5)

bool currMinUpdated = false;

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
bool isNight(DateTime const &time);
void setRTCtime(uint8_t, uint8_t);
void setText(Pattern);
int brightness_calc_next();

void buttonSingleClick()
{
  Serial.println("Button - Single click");
  // increase by a minute
  DateTime now = rtc.now();
  DateTime next = now + TimeSpan(0, 0, 1, 0);
  setRTCtime(next.hour(), next.minute());
}
void buttonDoubleClick()
{
  Serial.println("Button - Double click");
  // increase by an hour
  DateTime now = rtc.now();
  DateTime next = now + TimeSpan(0, 1, 0, 0);
  setRTCtime(next.hour(), next.minute());
}
void buttonLongPress()
{
  Serial.println("Button - Long press");
  // seek through: 5m per tick
  DateTime now = rtc.now();
  DateTime next = now + TimeSpan(0, 0, 5, 0);
  setRTCtime(next.hour(), next.minute());
}

// led index (row, column)
constexpr uint8_t ledIndex(uint8_t r, uint8_t c)
{
  return (r & 1)
             ? (r * COLS + (COLS - 1 - c)) // odd row: backward
             : (r * COLS + c);             // even row: forward
}

// pass in the coords of the left corner of the line
constexpr Pattern fromLine(uint8_t start_row, uint8_t start_column, uint8_t size, CRGB color)
{
  return (start_row & 1)
             ? Pattern{ledIndex(start_row, start_column) - size + 1, size, color} // backward (odd row)
             : Pattern{ledIndex(start_row, start_column), size, color};           // forward (even row)
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
const Pattern TEXT_1S = fromLine(5, 0, 3, CRGB::White); // just EIN (instead of EINS)
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
// minutes
const Pattern MINUTE_1 = fromLine(10, 0, 1, CRGB::White);
const Pattern MINUTE_2 = fromLine(10, 1, 1, CRGB::White);
const Pattern MINUTE_3 = fromLine(10, 2, 1, CRGB::White);
const Pattern MINUTE_4 = fromLine(10, 3, 1, CRGB::White);

void setup()
{
  Serial.begin(9600);
  Wire.begin();
  // the is no analogReadResolution, but it is 10 bit on this device

  Serial.println("Starting up...");

  // set up LED strip
  FastLED.addLeds<WS2812B, STRIP_DATA_PIN, COLOR_ORDER>(leds, NUM_LEDS);
  FastLED.setMaxPowerInMilliWatts(3500);
  FastLED.setBrightness(100);

  if (!rtc.begin(&Wire))
  {
    Serial.println("Couldn't find DS3231 RTC");
    setGridError(RTC_ERROR);
    // halt program
    while (1)
      delay(1000);
  }
  // check if there was a power loss since last rtc use
  if (rtc.lostPower()) // FIXME: always triggers, but keeps the time correctly?
  {
    Serial.println("RTC lost power, let's set the time!");
  }

  // setup button
  button.setup(
      BTN_PIN,      // Input pin for the button
      INPUT_PULLUP, // INPUT and enable the internal pull-up resistor
      true          // Active LOW (button pressed = LOW)
  );
  button.attachClick(buttonSingleClick);
  button.attachDoubleClick(buttonDoubleClick);
  button.attachDuringLongPress(buttonLongPress);
  button.setLongPressIntervalMs(250); // frequency to fire long press function if long press is detected

  Serial.println("Setup Complete");

  // "startup animation"
  fill_solid(leds, NUM_LEDS, CRGB::Blue);
  for (int i = 10; i < 100; i += 10)
  {
    leds[i] = CRGB::Red;
  }
  FastLED.show();
  delay(1000);
  fill_rainbow(leds, NUM_LEDS, 0, 255 / NUM_LEDS);
  FastLED.show();
  delay(1000);

  // display initial time
  DateTime now = rtc.now();
  setGridTime(now.hour(), now.minute());
}

void loop()
{
  // update buttons
  button.tick();

  // fetch current time from RTC module
  DateTime now = rtc.now();

  // adjust night light if needed
  int bright = brightness_calc_next();
  FastLED.setBrightness(bright);
  FastLED.show(); // need to call show to apply brightness change

  // update time
  if (now.second() == 0 && !currMinUpdated)
  {
    setGridTime(now.hour(), now.minute());
    currMinUpdated = true;
  }
  else if (now.second() > 0)
  {
    currMinUpdated = false;
  }

  // call multiple times a second to make sure we dont miss the minute increase
  delay(10);
}

static inline int brightness_map(int const &reading)
{
  int bright = (reading / 40) * (reading / 40) + 2;
  return constrain(bright, BRIGHTNESS_MIN, BRIGHTNESS_MAX);
}

// has smoothing included to avoid flickering
int brightness_calc_next()
{
  static const int BRIGHTNESS_HISTORY = 10;                 // number of past brightness values to consider
  static int brightness_values[BRIGHTNESS_HISTORY] = {100}; // array to store past brightness values
  static short brightness_index = 0;                        // index for the brightness array

  int val = analogRead(LDR_PIN);
  int b = brightness_map(val);
  // store the brightness value in the array
  brightness_values[brightness_index] = b;
  brightness_index = (brightness_index + 1) % BRIGHTNESS_HISTORY;
  // calculate the average of the last brightness values
  int sum = 0;
  for (int i = 0; i < BRIGHTNESS_HISTORY; i++)
  {
    sum += brightness_values[i];
  }
  return sum / BRIGHTNESS_HISTORY;
}

bool isNight(DateTime const &time)
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
  currMinUpdated = false;
}

void setGridTime(uint8_t hour, uint8_t minute)
{
  Serial.print("Setting time: ");
  Serial.print(hour);
  Serial.print(":");
  Serial.println(minute);

  // reset
  fill_solid(leds, NUM_LEDS, CRGB::Black);

  // modify minute for easy handling
  uint8_t rest = minute % 5;
  minute = minute - rest; // round to 5

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
    currHour = minute == 0 ? TEXT_1S : TEXT_1; // edge case for 1 o'clock, use EIN instead of EINS
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

  // set minute hand
  switch (rest)
  {
  case 1:
    setText(MINUTE_1);
    break;
  case 2:
    setText(MINUTE_1);
    setText(MINUTE_2);
    break;
  case 3:
    setText(MINUTE_1);
    setText(MINUTE_2);
    setText(MINUTE_3);
    break;
  case 4:
    setText(MINUTE_1);
    setText(MINUTE_2);
    setText(MINUTE_3);
    setText(MINUTE_4);
    break;
  }

  // update leds
  FastLED.show();
}

void setText(Pattern pattern)
{
  for (int i = pattern.start; i < pattern.start + pattern.length; i++)
  {
    leds[i] = pattern.color;
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
