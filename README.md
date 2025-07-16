# Arduino Clock In/Out System

A simple time tracking system built with Arduino that allows recording work hours using a push button. The system displays current time and status on a 16x2 LCD screen and stores clock in/out events in EEPROM for later retrieval.

## Features

- Real-time clock display with current date and time
- Single button operation for clock in/out
- Elapsed time display when clocked in
- Persistent storage of clock in/out events in EEPROM
- Serial output for reviewing historical records
- 16x2 LCD display with status indicators
- Simple long-press functionality to dump logs

## Hardware Requirements

- Arduino Uno
- 16x2 LCD Display
- DS1307 RTC Module
- Push Button
- 10kΩ Potentiometer (for LCD contrast)
- Resistor for LCD backlight (220Ω recommended)
- Jumper wires

## Wiring Instructions

### LCD Display Connection

| LCD Pin | Name     | Connect To         | Notes                                                  |
|---------|----------|-------------------|-------------------------------------------------------|
| 1       | VSS      | GND              | Power GND                                             |
| 2       | VDD      | 5V               | Power VCC                                             |
| 3       | VO       | 10kΩ pot middle  | Contrast. Other pot pins to 5V & GND                 |
| 4       | RS       | Arduino D7       | Register Select                                       |
| 5       | RW       | GND              | Always write (no reading needed)                      |
| 6       | E        | Arduino D6       | Enable                                                |
| 7-10    | D0-D3    | Not Connected    | Not used in 4-bit mode                               |
| 11      | D4       | Arduino D5       | Data bit 4                                           |
| 12      | D5       | Arduino D4       | Data bit 5                                           |
| 13      | D6       | Arduino D3       | Data bit 6                                           |
| 14      | D7       | Arduino D8       | Data bit 7                                           |
| 15      | A (LED+) | 5V via resistor  | Backlight power (use 220Ω resistor)                  |
| 16      | K (LED-) | GND              | Backlight ground                                      |

### Other Components

| Component       | Connection          | Notes                                    |
|----------------|--------------------|-----------------------------------------|
| Button         | D2 + GND           | Internal pull-up enabled in code        |
| RTC (DS1307)   | A4 (SDA), A5 (SCL) | Also connect VCC to 5V and GND to GND  |
| Serial Monitor | USB                | For viewing logs (9600 baud)            |

## Usage Instructions

1. **Normal Operation**
   - The display shows current date on the top line
   - Current time is shown on the bottom line
   - "OUT" indicator shows when clocked out
   - "IN" indicator and elapsed time show when clocked in

2. **Clocking In/Out**
   - Press the button briefly (< 5 seconds) to toggle between clock in/out
   - System stores each event with timestamp in EEPROM
   - Display shows elapsed time when clocked in

3. **Viewing Logs**
   - Long-press the button (≥ 5 seconds) to dump all records
   - Open Serial Monitor (9600 baud) to view the log
   - Records show chronologically with timestamps
   - Format: "Clock IN/OUT DD/MM/YYYY HH:MM:SS"
