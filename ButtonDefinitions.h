#pragma once

enum buttonName {
  Power,
  Up,
  Down,
  Left,
  Right,
  A,
  B,
  Start,
  Enter
};

struct Button {
  int pin;
  bool pressed;
  bool lastPressed;
};

Button buttons[] {
  { 35, false, false },
  { 13, false, false },
  { 32, false, false },
  { 33, false, false },
  { 22, false, false },
  { 26, false, false },
  { 27, false, false },
  { 14, false, false },
  { 21, false, false }
};

const int buttonCount = 9;
