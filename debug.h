#ifndef DEBUG_H
#define DEBUG_H
#define DEBUG 0
#include <Arduino.h>

class Debug {
 public:
  static void init() {
    if (DEBUG) {
      Serial.begin(9600);
    }
  }
  
  static void println(const char* message) {
    if (DEBUG) {
      Serial.println(message);
    }
  }
};

#endif