#include "button.h"

#include "debug.h"

Button::Button(int pin) {
  _pin = pin;

  pinMode(_pin, INPUT);
}

PressType Button::read() {}

PressType LongPressButton::read() {
  int reading = digitalRead(_pin);

  if (reading != _lastButtonState) {
    _lastDebounceTime = millis();
    _lastButtonState = reading;
  }

  if (reading && _pressStatus == PRESS_STATUS_ACTIVATED) {
    if ((millis() - _lastDebounceTime) > LONG_PRESS_DELAY) {
      _pressStatus = PRESS_STATUS_LONG;
      Debug::println("long press");
      return PRESS_TYPE_SECONDARY;
    }

    return PRESS_TYPE_NONE;
  }

  if ((millis() - _lastDebounceTime) > DEBOUNCE_DELAY) {
    if (reading == LOW) {
      if (_pressStatus == PRESS_STATUS_ACTIVATED) {
        Debug::println("secondary action");
        _pressStatus = PRESS_STATUS_NONE;
        return PRESS_TYPE_PRIMARY;
      }

      _pressStatus = PRESS_STATUS_NONE;
    } else {
      if (_pressStatus == PRESS_STATUS_NONE) {
        _lastDebounceTime = millis();
        _pressStatus = PRESS_STATUS_ACTIVATED;
      }
    }
  }

  return PRESS_TYPE_NONE;
}

PressType RepeatButton::read() {
  int reading = digitalRead(_pin);

  if (reading != _lastButtonState) {
    _lastDebounceTime = millis();
    _lastButtonState = reading;
  }

  if (reading && _pressStatus == PRESS_STATUS_ACTIVATED) {
    if ((millis() - _lastDebounceTime) > REPEAT_DELAY) {
      _lastDebounceTime = millis();
      Debug::println("repeat press");
      return PRESS_TYPE_PRIMARY;
    }

    return PRESS_TYPE_NONE;
  }

  if ((millis() - _lastDebounceTime) > DEBOUNCE_DELAY) {
    if (reading == HIGH && _pressStatus == PRESS_STATUS_NONE) {
      _lastDebounceTime = millis();
      _pressStatus = PRESS_STATUS_ACTIVATED;

      Debug::println("repeat first press");
      return PRESS_TYPE_PRIMARY;
    }
  }

  if (reading == LOW) {
    _pressStatus = PRESS_STATUS_NONE;
  }

  return PRESS_TYPE_NONE;
}