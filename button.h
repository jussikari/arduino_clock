#ifndef button_h
#define button_h
#include <Arduino.h>

const int DEBOUNCE_DELAY = 30;
const int REPEAT_DELAY = 200;
const int LONG_PRESS_DELAY = 1000;

enum PressType {
  PRESS_TYPE_NONE = 0,
  PRESS_TYPE_PRIMARY = 1,
  PRESS_TYPE_SECONDARY = 2
};

enum PressStatus {
  PRESS_STATUS_NONE = 0,
  PRESS_STATUS_ACTIVATED = 1,
  PRESS_STATUS_LONG = 2
};

class Button {
 public:
  Button(int pin);
  virtual PressType read();

 protected:
  int _pin;
  int _lastButtonState = LOW;
  unsigned long _lastDebounceTime = 0;
  PressStatus _pressStatus = PRESS_STATUS_NONE;
};

class LongPressButton : Button {
 public:
  LongPressButton(int pin) : Button(pin){};
  PressType read() override;
};

class RepeatButton : Button {
 public:
  RepeatButton(int pin) : Button(pin){};
  PressType read() override;
};

#endif