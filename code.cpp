#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include "RTClib.h"

LiquidCrystal_I2C lcd(0x27, 16, 2);
RTC_DS3231 rtc;

#define RELAY_PIN 18
#define BUTTON_PIN 19

bool bellActive = false;
unsigned long bellStart = 0;
int lastMinute = -1;
unsigned long lastLCDUpdate = 0;

// =============================
// SET YOUR BELL SCHEDULE HERE
// =============================
struct BellTime {
  int hour;
  int minute;
  bool triggered;
};

BellTime schedule[] = {
  {8,  0,  false},  // 08:00 AM
  {10, 0,  false},  // 10:00 AM
  {12, 0,  false},  // 12:00 PM
  {14, 0,  false},  // 02:00 PM
  {16, 0,  false},  // 04:00 PM
};
int scheduleCount = sizeof(schedule) / sizeof(schedule[0]);
// =============================

void startBell() {
  digitalWrite(RELAY_PIN, LOW);   // LOW = relay ON
  bellActive = true;
  bellStart = millis();
}

void stopBell() {
  digitalWrite(RELAY_PIN, HIGH);  // HIGH = relay OFF
  bellActive = false;
}

void setup() {
  Wire.begin(21, 22);
  lcd.init();
  lcd.backlight();

  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, HIGH); // relay OFF at startup

  pinMode(BUTTON_PIN, INPUT_PULLUP);

  rtc.begin();
  // rtc.adjust(DateTime(F(__DATE__), F(__TIME__))); // uncomment ONCE to set time

  // Show schedule count on LCD at startup
  lcd.setCursor(0, 0);
  lcd.print("School Bell Sys ");
  lcd.setCursor(0, 1);
  lcd.print("Schedules: ");
  lcd.print(scheduleCount);
  lcd.print("     ");
  delay(2000);
  lcd.clear();
}

void loop() {
  DateTime now = rtc.now();

  // --- Update LCD every 500ms ---
  if (millis() - lastLCDUpdate > 500) {
    lastLCDUpdate = millis();

    // Row 0: current time
    lcd.setCursor(0, 0);
    if (now.hour() < 10) lcd.print("0");
    lcd.print(now.hour());
    lcd.print(":");
    if (now.minute() < 10) lcd.print("0");
    lcd.print(now.minute());
    lcd.print(":");
    if (now.second() < 10) lcd.print("0");
    lcd.print(now.second());
    lcd.print("   ");

    // Row 1: status + next bell
    lcd.setCursor(0, 1);
    if (bellActive) {
      lcd.print("BELL RINGING!   ");
    } else {
      // find next bell time
      int nextHour = -1, nextMinute = -1;
      for (int i = 0; i < scheduleCount; i++) {
        int schedMins = schedule[i].hour * 60 + schedule[i].minute;
        int nowMins = now.hour() * 60 + now.minute();
        if (schedMins > nowMins) {
          if (nextHour == -1 || schedMins < nextHour * 60 + nextMinute) {
            nextHour = schedule[i].hour;
            nextMinute = schedule[i].minute;
          }
        }
      }
      if (nextHour != -1) {
        lcd.print("Next:");
        if (nextHour < 10) lcd.print("0");
        lcd.print(nextHour);
        lcd.print(":");
        if (nextMinute < 10) lcd.print("0");
        lcd.print(nextMinute);
        lcd.print("      ");
      } else {
        lcd.print("Done 4 today!   ");
      }
    }
  }

  // --- Reset triggered flags when minute changes ---
  if (now.minute() != lastMinute) {
    lastMinute = now.minute();
    for (int i = 0; i < scheduleCount; i++) schedule[i].triggered = false;
  }

  // --- Check schedule ---
  for (int i = 0; i < scheduleCount; i++) {
    if (now.hour() == schedule[i].hour &&
        now.minute() == schedule[i].minute &&
        !schedule[i].triggered) {
      schedule[i].triggered = true;
      startBell();
    }
  }

  // --- Physical button: stop if ringing, ring if idle ---
  if (digitalRead(BUTTON_PIN) == LOW) {
    if (bellActive) {
      stopBell();
    } else {
      startBell();
    }
    delay(300); // debounce
  }

  // --- Auto stop after 5 seconds ---
  if (bellActive && millis() - bellStart > 5000) {
    stopBell();
  }
}
