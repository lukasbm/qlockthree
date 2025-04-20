#include <FastLED.h>
#include <RTClib.h>

// led index(row, col)
#define LIX(r, c) (((r)&1) ? ((r)*WIDTH + (WIDTH - 1 - (c))) : ((r)*WIDTH + (c)))

uint8_t ledIndex(uint8_t row, uint8_t col) {
  if (y & 1) {  // same as y % 2 == 1 but faster
    // odd rows run backwards
    return (row * COLS) + (COLS - 1 - col);
  } else {
    // even rows run forwards
    return (row * COLS) + col;
  }
}
