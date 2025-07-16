#include <Wire.h>
#include <LiquidCrystal.h>
#include <RTClib.h>
#include <EEPROM.h>

/***************************************************************************************************
| LCD Pin | Name     | Connect To         | Notes                                                  |
| ------- | -------- | ------------------ | ------------------------------------------------------ |
| 1       | VSS      | GND                | Power GND                                              |
| 2       | VDD      | 5V                 | Power VCC                                              |
| 3       | VO       | Middle of 10k pot  | Contrast. Other legs to 5V & GND (or fixed ~2k to GND) |
| 4       | RS       | Arduino **D7**     | Register Select                                        |
| 5       | RW       | GND                | Always write (no reading needed)                       |
| 6       | E        | Arduino **D6**     | Enable                                                 |
| 7       | D0       | —                  | Not used in 4-bit mode                                 |
| 8       | D1       | —                  | Not used in 4-bit mode                                 |
| 9       | D2       | —                  | Not used in 4-bit mode                                 |
| 10      | D3       | —                  | Not used in 4-bit mode                                 |
| 11      | D4       | Arduino **D5**     | Data bit 4                                             |
| 12      | D5       | Arduino **D4**     | Data bit 5                                             |
| 13      | D6       | Arduino **D3**     | Data bit 6                                             |
| 14      | D7       | Arduino **D8**     | Data bit 7                                             |
| 15      | A (LED+) | 5V (with resistor) | Backlight power                                        |
| 16      | K (LED−) | GND                | Backlight ground                                       |
***************************************************************************************************/

/******************************************************************************
| Component              | Pin on Arduino Uno | Notes                         |
| --------------         | ------------------ | ----------------------------- |
| Button                 | D2                 | With internal pull-up enabled |
|                        | Other leg          | GND                           |
| Serial logging         | USB                | Serial Monitor                |
|                        | GND                | GND                           |
| **RTC (DS1307)**       | SDA                | A4                            |
|                        | SCL                | A5                            |
|                        | VCC                | 5V                            |
|                        | GND                | GND                           |
| **(Optional pull-up)** |                    | Use `INPUT_PULLUP` in code    |
| **EEPROM** (optional)  | SDA/SCL            | A4 / A5 (same I2C bus)        |
******************************************************************************/

// LCD pins: RS, E, D4, D5, D6, D7
LiquidCrystal lcd(7, 6, 5, 4, 3, 8);
RTC_DS1307 rtc;

const int buttonPin = 2;
bool buttonPressed = false;
unsigned long buttonPressTime = 0;
const unsigned long debounceDelay = 50;
const unsigned long longPressThreshold = 5000; // 5 seconds

bool clockedIn = false;
DateTime clockInTime;
int eepromAddress = 0;

// Setup printf() support on Arduino Uno
int serial_putchar(char c, FILE *) {
  Serial.write(c);
  return c;
}
FILE serial_stdout;

void setup() {
  pinMode(buttonPin, INPUT_PULLUP);
  lcd.begin(16, 2);
  Serial.begin(9600);

  // Enable printf()
  fdev_setup_stream(&serial_stdout, serial_putchar, NULL, _FDEV_SETUP_WRITE);
  stdout = &serial_stdout;

  if (!rtc.begin()) {
    lcd.print("RTC Error!");
    while (1);
  }

  if (!rtc.isrunning()) {
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  }

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("System Ready");
  delay(1000);
}

void loop() {
  DateTime now = rtc.now();

  lcd.setCursor(0, 0);
  lcd.print(now.day() < 10 ? "0" : ""); lcd.print(now.day()); lcd.print("/");
  lcd.print(now.month() < 10 ? "0" : ""); lcd.print(now.month()); lcd.print("/");
  lcd.print(now.year());

  lcd.setCursor(0, 1);
  lcd.print(now.hour() < 10 ? "0" : ""); lcd.print(now.hour()); lcd.print(":");
  lcd.print(now.minute() < 10 ? "0" : ""); lcd.print(now.minute()); lcd.print(":");
  lcd.print(now.second() < 10 ? "0" : ""); lcd.print(now.second());

  handleButton(now);

  // Display elapsed time if clocked in
  if (clockedIn) {
    unsigned long elapsed = now.unixtime() - clockInTime.unixtime();
    int h = elapsed / 3600;
    int m = (elapsed % 3600) / 60;
    int s = elapsed % 60;

    lcd.setCursor(10, 0);
    lcd.print("  IN ");

    lcd.setCursor(10, 1);
    lcd.print(h < 10 ? "0" : ""); lcd.print(h); lcd.print(":");
    lcd.print(m < 10 ? "0" : ""); lcd.print(m); lcd.print(":");
    //lcd.print(s < 10 ? "0" : ""); lcd.print(s);
  } else {
    lcd.setCursor(10, 0);
    lcd.print("  OUT");
    lcd.setCursor(9, 1);
    lcd.print("         ");
  }

  delay(250);
}

void handleButton(DateTime now) {
  bool reading = !digitalRead(buttonPin); // Active LOW

  if (reading && !buttonPressed) {
    // Button just pressed
    buttonPressed = true;
    buttonPressTime = millis();
  }

  if (!reading && buttonPressed) {
    // Button just released
    unsigned long pressDuration = millis() - buttonPressTime;
    buttonPressed = false;

    if (pressDuration >= longPressThreshold) {
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Dumping logs...");
      dumpEEPROMRecords();
      delay(1500); // allow user to read the message
    } else if (pressDuration >= debounceDelay) {
      handleClockToggle();
    }
  }
}

void handleClockToggle() {
  DateTime now = rtc.now();

  if (clockedIn) {
    writeEventToEEPROM(false, now);
    clockedIn = false;
  } else {
    clockInTime = now;
    writeEventToEEPROM(true, now);
    clockedIn = true;
  }
}

void writeEventToEEPROM(bool isClockIn, DateTime time) {
  EEPROM.update(eepromAddress++, isClockIn ? 1 : 0);
  EEPROM.update(eepromAddress++, time.year() - 2000);
  EEPROM.update(eepromAddress++, time.month());
  EEPROM.update(eepromAddress++, time.day());
  EEPROM.update(eepromAddress++, time.hour());
  EEPROM.update(eepromAddress++, time.minute());
  EEPROM.update(eepromAddress++, time.second());
}

void dumpEEPROMRecords() {
  printf("\n== Clock In/Out Log Dump ==\n");
  int addr = 0;

  while (addr + 7 <= EEPROM.length()) {
    byte type = EEPROM.read(addr++);
    byte yr = EEPROM.read(addr++) + 2000;
    byte mo = EEPROM.read(addr++);
    byte dy = EEPROM.read(addr++);
    byte hr = EEPROM.read(addr++);
    byte mi = EEPROM.read(addr++);
    byte se = EEPROM.read(addr++);

    if (yr > 2100 || mo == 0 || mo > 12) break;

    printf("%s %02d/%02d/%04d %02d:%02d:%02d\n",
           type ? "Clock IN " : "Clock OUT",
           dy, mo, yr, hr, mi, se);
  }
}
