#include <Arduino.h>

#include "MD_MAX72xx.h"
#include "MD_Parola.h"
#include "RTClib.h"
#include "SPI.h"

#include "font_data.h"
#include "button.h"
#include "debug.h"

// MAX7219 hardware type, size, and output pins:
#define HARDWARE_TYPE MD_MAX72XX::FC16_HW
#define MAX_DEVICES 4
#define CS_PIN 10
#define DATA_PIN 12
#define CLK_PIN 11

#define PIN_MODE 7
#define PIN_UP 6
#define PIN_DOWN 5

// Milliseconds between blinking digits in edit mode
#define BLINK_INTERVAL 200

// Milliseconds between changing time and date in alternate mode
#define ALTERNATE_INTERVAL 30000

enum State {
  STATE_INIT = 0,
  STATE_TIME = 1,
  STATE_DATE = 2,
  STATE_ALTERNATE = 3,
  STATE_EDIT
};
enum DisplayMode { TIME = 1, DATE = 2 };
enum EditMode { NONE = 0, MINUTE = 1, HOUR = 2, DAY = 3, MONTH = 4, YEAR = 5 };
struct NextView {
  bool isPending = true;
  textEffect_t effectIn = PA_SCROLL_DOWN;
  State state = STATE_TIME;
  DisplayMode displayMode = DisplayMode::TIME;
  bool showText = false;
} next;

LongPressButton modeButton(PIN_MODE);
RepeatButton upButton(PIN_UP);
RepeatButton downButton(PIN_DOWN);

int lastMinute = 0;
State state = State::STATE_INIT;
DisplayMode displayMode;

EditMode editMode = EditMode::NONE;
char displayString[10];
char hourString[3];
char minuteString[3];
char yearString[3];
char monthString[3];
char dayString[3];
unsigned long lastAlternateMillis = 0;

MD_Parola display = MD_Parola(HARDWARE_TYPE, CS_PIN, MAX_DEVICES);
RTC_DS1307 rtc;

void setString(char* str, EditMode modeMatch, int value) {
  int blink = (millis() / BLINK_INTERVAL) % 2;

  if (editMode != modeMatch || blink == 1) {
    sprintf(str, "%02d", value);
  } else {
    sprintf(str, "  ", value);
  }
}

void drawTime(textEffect_t in = PA_PRINT, textEffect_t out = PA_NO_EFFECT) {
  DateTime now = rtc.now();

  setString(hourString, EditMode::HOUR, now.hour());
  setString(minuteString, EditMode::MINUTE, now.minute());

  sprintf(displayString, "%s:%s", hourString, minuteString);
  display.setTextAlignment(PA_CENTER);
  display.setFont(0, largeDigitDont);
  display.displayText(displayString, display.getTextAlignment(),
                      display.getSpeed(), 0, in, out);
}

void drawDate(textEffect_t in = PA_PRINT, textEffect_t out = PA_NO_EFFECT) {
  DateTime now = rtc.now();

  setString(yearString, EditMode::YEAR, now.year() % 100);
  setString(monthString, EditMode::MONTH, now.month());
  setString(dayString, EditMode::DAY, now.day());

  sprintf(displayString, "%s.%s.-%s", dayString, monthString, yearString);
  display.setTextAlignment(PA_CENTER);
  display.setFont(0, smallDigitFont);
  display.displayText(displayString, display.getTextAlignment(),
                      display.getSpeed(), 0, in, out);
}

int numberOfDaysInMonth(int year, int month) {
  switch (month) {
    case 1:   // January
    case 3:   // March
    case 5:   // May
    case 7:   // July
    case 8:   // August
    case 10:  // October
    case 12:  // December
      return 31;
    case 4:   // April
    case 6:   // June
    case 9:   // September
    case 11:  // November
      return 30;
    case 2:  // February
      if ((year % 4 == 0 && year % 100 != 0) || year % 400 == 0) {
        return 29;
      } else {
        return 28;
      }
    default:
      return 0;
  }
}

void setup() {
  if (!rtc.begin()) {  // start the RTC module
    abort();
  }

  Debug::init();

  display.begin();
  display.setFont(0, nullptr);
  display.setIntensity(8); //0-15
  display.setPause(6);
  display.displayClear();

  display.displayText("Papan kello", PA_RIGHT, 130,
                      0, PA_SCROLL_LEFT, PA_SCROLL_LEFT);
}

void changeTime(int change) {
  DateTime now = rtc.now();

  if (editMode == EditMode::MINUTE) {
    rtc.adjust(DateTime(now.year(), now.month(), now.day(), now.hour(),
                        (now.minute() + change + 60) % 60, 0));

  } else if (editMode == EditMode::HOUR) {
    rtc.adjust(DateTime(now.year(), now.month(), now.day(),
                        (now.hour() + change + 24) % 24, now.minute(), 0));
  } else if (editMode == EditMode::DAY) {
    int daysInMonth = numberOfDaysInMonth(now.year(), now.month());
    rtc.adjust(
        DateTime(now.year(), now.month(),
                 (now.day() - 1 + change + daysInMonth) % daysInMonth + 1,
                 now.hour(), now.minute(), 0));
  } else if (editMode == EditMode::MONTH) {
    rtc.adjust(DateTime(now.year(), (now.month() - 1 + 12 + change) % 12 + 1,
                        now.day(), now.hour(), now.minute(), 0));
  } else if (editMode == EditMode::YEAR) {
    rtc.adjust(DateTime(2000 + (now.year() + change) % 100, now.month(),
                        now.day(), now.hour(), now.minute(), 0));
  }
}

void loop() {
  PressType modeButtonPress = modeButton.read();
  PressType upButtonPress = upButton.read();
  PressType downButtonPress = downButton.read();

  if (!next.isPending) {
    if (modeButtonPress == PRESS_TYPE_PRIMARY) {
      if (editMode == EditMode::NONE) {
        changeMode();
      } else {
        editMode = (EditMode)((editMode + 1) % 6);

        // Force redraw of current mode when exit editing
        if (editMode == EditMode::NONE) {
          lastMinute = -1;
        }
      }
    } else if (modeButtonPress == PRESS_TYPE_SECONDARY) {
      editMode = EditMode::MINUTE;
    }
  }

  DateTime now = rtc.now();

  if (editMode != EditMode::NONE && (upButtonPress == PRESS_TYPE_PRIMARY ||
                                     downButtonPress == PRESS_TYPE_PRIMARY)) {
    int change = upButtonPress == PRESS_TYPE_PRIMARY ? 1 : -1;
    changeTime(change);
  }

  if (display.displayAnimate()) {
    if (next.isPending) {
      if (next.showText) {
        char* text;
        if (next.state == STATE_TIME) {
          text = "Klo";
        } else if (next.state == STATE_DATE) {
          text = "Pvm";
        } else {
          text = "Vaihto";
        }

        display.setFont(0, nullptr);
        display.displayText(text, display.getTextAlignment(),
                            display.getSpeed(), 500, PA_OPENING_CURSOR,
                            PA_OPENING_CURSOR);

        next.showText = false;

      } else {
        display.setPause(0);
        display.setSpeed(20);
        state = next.state;
        displayMode = next.displayMode;

        if (next.displayMode == DisplayMode::TIME) {
          drawTime(next.effectIn, PA_NO_EFFECT);
        } else {
          drawDate(next.effectIn, PA_NO_EFFECT);
        }

        next.isPending = false;
      }
      return;
    }

    if (editMode != EditMode::NONE) {
      if (editMode == EditMode::HOUR || editMode == EditMode::MINUTE) {
        drawTime(PA_PRINT, PA_NO_EFFECT);
      } else {
        drawDate(PA_PRINT, PA_NO_EFFECT);
      }

      return;
    }

    if (lastMinute != now.minute()) {
      if (displayMode == TIME) {
        drawTime();
      } else {
        drawDate();
      }
    }

    if (state == STATE_ALTERNATE) {
      if (millis() - lastAlternateMillis > ALTERNATE_INTERVAL) {
        lastAlternateMillis = millis();
        textEffect_t effect =
            displayMode == TIME ? PA_SCROLL_UP : PA_SCROLL_DOWN;
        if (displayMode == TIME) {
          drawTime(PA_PRINT, effect);
        } else {
          drawDate(PA_PRINT, effect);
        }
        next.effectIn = effect;
        next.isPending = true;
        next.showText = false;
        next.displayMode =
            displayMode == TIME ? DisplayMode::DATE : DisplayMode::TIME;
      }
    }

    lastMinute = now.minute();
  }
}

void changeMode() {
  if (displayMode == DisplayMode::TIME) {
    drawTime(PA_PRINT, PA_OPENING_CURSOR);
  } else {
    drawDate(PA_PRINT, PA_OPENING_CURSOR);
  }

  if (state == STATE_TIME) {
    next.state = STATE_DATE;
  } else if (state == STATE_DATE) {
    next.state = STATE_ALTERNATE;
  } else {
    next.state = STATE_TIME;
  }

  next.displayMode =
      next.state == STATE_DATE ? DisplayMode::DATE : DisplayMode::TIME;
  next.effectIn = PA_OPENING_CURSOR;
  next.isPending = true;
  next.showText = true;
  lastAlternateMillis = millis();
}