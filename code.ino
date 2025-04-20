
// led index(row, col)

uint8_t ledIndex(uint8_t row, uint8_t col) {
  if (y & 1) {  // same as y % 2 == 1 but faster
    // odd rows run backwards
    return (row * COLS) + (COLS - 1 - col);
  } else {
    // even rows run forwards
    return (row * COLS) + col;
  }
}
