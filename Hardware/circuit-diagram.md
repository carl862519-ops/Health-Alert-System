# CIRCUIT DIAGRAM - HEALTH ALERT SYSTEM

## Arduino Pin Connections

### Inputs (Sensors):
1. **IR Heart Rate Sensor**: Pin 2 (with interrupt capability)
2. **Tilt Switch**: Pin 3 (digital input)
3. **Fluid Level Sensor**: Pin 4 (digital input)

### Outputs (Actuators):
4. **Buzzer**: Pin 8
5. **Red LED**: Pin 5
6. **Green LED**: Pin 6
7. **Blue LED**: Pin 7

## Sensor Configurations

### IR Heart Rate Sensor:
```
IR Sensor VCC → 5V
IR Sensor GND → GND
IR Sensor OUT → Pin 2 (Arduino)
           → 10kΩ resistor → GND (pull-down)
```

### Tilt Switch:
```
5V → Tilt Switch → 10kΩ resistor → GND
                → Pin 3 (Arduino) - with internal pull-up
```

### Fluid Level Sensor:
```
5V → Fluid Sensor VCC
GND → Fluid Sensor GND
Sensor OUT → Pin 4 (Arduino)

Note: Calibrate based on fluid conductivity
```

### RGB LED (Traffic Light Indicator):
```
Pin 5 → 220Ω → Red LED → GND
Pin 6 → 220Ω → Green LED → GND
Pin 7 → 220Ω → Blue LED → GND

Common Cathode configuration
Current limiting resistors protect LEDs
```

## Power Requirements
- **Primary**: USB 5V (500mA max)
- **Sensors**: All powered from Arduino 5V rail
- **Total Current**: <400mA
- **Backup Option**: 9V battery via DC jack

## Safety Considerations
1. **Isolation**: No direct patient contact with circuit
2. **Voltage**: All components use safe 5V levels
3. **Current**: Limited to prevent any hazard
4. **Enclosure**: Recommended for clinical use
