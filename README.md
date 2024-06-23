
# Heart Rate Monitoring System with PIC16F877A

This project implements a heart rate monitoring system using the PIC16F877A microcontroller. The system measures the heart rate (BPM) and the time interval between beats (IBI) and displays them on an LCD. Additionally, an RGB LED indicates the heart rate status, and a buzzer provides auditory feedback.

## Components

- PIC16F877A Microcontroller
- LCD Display
- RGB LED
- Buzzer
- ADC Module
- Timer0 Module
- Heartbeat sensor (connected to ADC)
- Supporting C files for LCD, EUSART, ADC, and Timer0

## Configuration Bits

```c
#pragma config FOSC = HS        // Oscillator Selection bits (HS oscillator)
#pragma config WDTE = OFF       // Watchdog Timer Enable bit (WDT disabled)
#pragma config PWRTE = OFF      // Power-up Timer Enable bit (PWRT disabled)
#pragma config BOREN = ON       // Brown-out Reset Enable bit (BOR enabled)
#pragma config LVP = OFF        // Low-Voltage (Single-Supply) In-Circuit Serial Programming Enable bit (RB3/PGM pin has PGM function; low-voltage programming enabled)
#pragma config CPD = OFF        // Data EEPROM Memory Code Protection bit (Data EEPROM code protection off)
#pragma config WRT = OFF        // Flash Program Memory Write Enable bits (Write protection off; all program memory may be written to by EECON control)
#pragma config CP = OFF         // Flash Program Memory Code Protection bit (Code protection off)
```

## Pin Configuration

- RB3: Buzzer Output
- RC5: RGB LED Red
- RC6: RGB LED Green
- RC7: RGB LED Blue
- ADC Channel 0: Heartbeat Sensor Input

## System Initialization

The `system_init` function configures the I/O pins, initializes the LCD, ADC, Timer0, and interrupts.

## Main Program Flow

1. **Reading ADC Value:** The system continuously reads the ADC value from the heartbeat sensor.
2. **Calculating Heartbeat:** Based on the ADC values, the system calculates the BPM and IBI.
3. **Displaying Values:** The calculated BPM and IBI values are displayed on the LCD.
4. **Buzzer Control:** The buzzer provides feedback based on the BPM value.
5. **RGB LED Control:** The RGB LED changes color based on the BPM value.

## Functions

### `system_init`

Initializes the system, including I/O pins, LCD, ADC, Timer0, and interrupts.

### `calculate_heart_beat(int adc_value)`

Calculates the heart rate and IBI based on the ADC values.

### `buzzer_control(bool is_high)`

Controls the buzzer frequency based on the heart rate.

### `rgb_led_control(int bpm)`

Controls the RGB LED color based on the heart rate.

## Interrupt Service Routine

Handles Timer0 interrupts to periodically read the ADC value.

## Supporting C Files

- `lcd.c`: Functions to initialize and control the LCD display.
- `eusart1.c`: Functions for EUSART communication (not used in this project).
- `adc.c`: Functions to initialize and read values from the ADC module.
- `tmr0.c`: Functions to initialize and control Timer0.

## Usage

1. Connect the components as described.
2. Compile and upload the code to the PIC16F877A microcontroller.
3. The system will display the heart rate and IBI values on the LCD.
4. The RGB LED and buzzer will provide feedback based on the heart rate.

## Notes

- Ensure that the heartbeat sensor is properly connected to the ADC channel.
- Adjust the threshold values in `calculate_heart_beat` function if necessary to optimize heart rate detection.

## References

Some parts of the code were adapted from the following sources:
1. [Heart rate monitoring system using Arduino in Proteus 8 Professional by Tejeswar Rao](https://github.com/Tejeswar-Rao/Heart-rate-monitoring-system-using-Arduino-in-Proteus-8-Professional/blob/main/source_main.ino)
2. [Gist by Electronza](https://gist.github.com/Electronza/978d8d890e28f66293303f6e9ee6e626)

## License

This project is open-source and available for modification and distribution under the MIT License.
