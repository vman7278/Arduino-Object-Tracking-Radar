# Arduino Object-Tracking Radar 📡

An Arduino-based system that scans for nearby objects and approximately follows their horizontal movement using an **HC-SR04 ultrasonic distance sensor** and **SG90 servo**.

This is my **second documented Arduino project**, after an LED reaction-time tester. I built a temporary cardboard mounting bracket to attach the ultrasonic sensor to the rotating servo arm.

## Features

- Scans across an approximately 20°–160° range to locate nearby objects.
- Tracks possible targets roughly 8–65 cm away by checking adjacent directions.
- Reacquires nearby targets after losing a valid reading.
- Prints the tracking angle and latest measured distance in the **Arduino IDE Serial Monitor**.
- Offers manual center (“zero”) adjustment using Serial Monitor commands.

**Limitations:** An HC-SR04 measures distance, not object identity or direction. To estimate horizontal movement, the servo must make small left/right checks. Objects closer to the sensor may distract it, and motion will not be as smooth as camera tracking.

## Hardware

- Arduino Uno (ELEGOO-compatible)
- HC-SR04 ultrasonic distance sensor
- SG90 micro servo
- Breadboard and jumper wires
- Cardboard and tape for a temporary sensor mount
- USB cable and Arduino IDE

The **LCD1602 screen is not connected** in the current build because it requires additional wiring. Distance is displayed on the computer instead.

## Wiring

| Component | Connection |
| --- | --- |
| SG90 orange (signal) | Arduino D4 |
| SG90 red (power) | Arduino 5V rail |
| SG90 brown (ground) | Arduino GND rail |
| HC-SR04 TRIG | Arduino D9 |
| HC-SR04 ECHO | Arduino D10 |
| HC-SR04 VCC | 5V rail |
| HC-SR04 GND | GND rail |

Connect the breadboard positive rail to Arduino 5V and negative rail to Arduino GND. Verify that any split breadboard power rails are connected appropriately.

## Run it

1. Open [Object_Tracking_Radar.ino](Object_Tracking_Radar.ino) in Arduino IDE.
2. Select the Arduino Uno and the correct USB port.
3. Upload the sketch.
4. Open **Tools → Serial Monitor** at **9600 baud**.
5. Hold a flat book approximately 20–30 cm in front of the sensor and slowly move it sideways.

Example output (illustrative):

~~~text
SEARCHING...
TRACKING | Angle: 90 deg | Distance: 24.3 cm
TRACKING | Angle: 95 deg | Distance: 26.1 cm
TRACKING | Angle: 95 deg | Distance: no valid center reading
TARGET LOST
SEARCHING...
~~~

The distance reading is measured at the current estimated tracking direction. A missing reading is reported as such, not as a zero distance.

## Manual center (“zero”) calibration

Open the Arduino IDE Serial Monitor at **9600 baud** and send:

| Command | Action |
| --- | --- |
| `z` | Pause tracking and point to the center |
| `+` | Adjust center one degree in one direction |
| `-` | Adjust center one degree in the other direction |
| `r` | Resume scanning and tracking |

The calibration is restricted to servo center values from 70° to 110°, keeping the configured sweep within its nominal 0°–180° travel. If alignment requires more than that, turn off power and reposition the servo horn rather than forcing the servo.

Calibration changes are temporary. To preserve the preferred center after restarting, change the `zeroPosition` variable at the top of the sketch from its default value of 90.

## Tracking logic

1. **Search:** scan the area to find the closest reading within the detection range.
2. **Check:** take measurements at the current target angle and 12° to either side.
3. **Adjust:** if only one side detects an object, step 5° in that direction.
4. **Recover:** after three consecutive cycles with no valid nearby reading, search the wider area again.
5. **Report:** print the current angle and distance to Serial Monitor.

## Notes and future improvements

- Add a 16×2 LCD once the wiring and contrast control are ready.
- Replace the temporary cardboard mount with a sturdier bracket.
- Experiment with filtering multiple ultrasonic measurements to improve stability.
- Build an on-screen angular radar visualization.

**Safety:** Disconnect USB before rewiring. Do not connect a 9V battery directly to the 5V sensor, servo, or Arduino 5V rail. If the servo stalls, the Arduino resets, or the computer reports a USB power warning, disconnect and use an appropriately regulated 5V supply for the servo, with common ground. Keep sensor wires clear of moving parts.
