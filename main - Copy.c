#pragma config FOSC = HS        // Oscillator Selection bits (HS oscillator)
#pragma config WDTE = OFF       // Watchdog Timer Enable bit (WDT disabled)
#pragma config PWRTE = OFF      // Power-up Timer Enable bit (PWRT disabled)
#pragma config BOREN = ON       // Brown-out Reset Enable bit (BOR enabled)
#pragma config LVP = OFF        // Low-Voltage (Single-Supply) In-Circuit Serial Programming Enable bit (RB3/PGM pin has PGM function; low-voltage programming enabled)
#pragma config CPD = OFF        // Data EEPROM Memory Code Protection bit (Data EEPROM code protection off)
#pragma config WRT = OFF        // Flash Program Memory Write Enable bits (Write protection off; all program memory may be written to by EECON control)
#pragma config CP = OFF         // Flash Program Memory Code Protection bit (Code protection off)

#include <xc.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include "supporing_cfile/lcd.h"
#include "supporing_cfile/eusart1.h"
#include "supporing_cfile/adc.h"
#include "supporing_cfile/tmr0.h"

/*
Hardware related definition
*/
#define _XTAL_FREQ 20000000 // Crystal Frequency, used in delay

/*
Program Flow related definition
*/
 

#define READ_ADC  1
#define CALCULATE_HEART_BEAT 2
#define SHOW_HEART_BEAT 3
#define IDLE 0
#define DEFAULT -1

volatile int rate[10]; // array to hold last ten IBI values
volatile unsigned long sampleCounter = 0; // used to determine pulse timing
volatile unsigned long lastBeatTime = 0; // used to find IBI
volatile int P = 512; // used to find peak in pulse wave, seeded
volatile int T = 512; // used to find trough in pulse wave, seeded
volatile int thresh = 530; // used to find instant moment of heart beat, seeded
volatile int amp = 0; // used to hold amplitude of pulse waveform, seeded
volatile bool firstBeat = true; // used to seed rate array so we startup with reasonable BPM
volatile bool secondBeat = false; // used to seed rate array so we startup with reasonable BPM

volatile int BPM; // int that holds raw Analog in 0. updated every 2mS
volatile int Signal; // holds the incoming raw data
volatile int IBI = 600; // int that holds the time interval between beats! Must be seeded!
volatile bool Pulse = false; // "True" when User's live heartbeat is detected. "False" when not a "live beat".
volatile bool QS = false; // becomes true when finds a beat.

int main_state = -1;
int adc_value = 0;



int tune = 0;
/*
Other Specific definition
*/
void system_init(void);

void calculate_heart_beat(int adc_value) {
    Signal = adc_value;
    sampleCounter += 2; // keep track of the time in mS with this variable
    int N = sampleCounter - lastBeatTime; // monitor the time since the last beat to avoid noise

    //  find the peak and trough of the pulse wave
    if (Signal < thresh && N > (IBI / 5) * 3) { // avoid dichrotic noise by waiting 3/5 of last IBI
        if (Signal < T) { // T is the trough
            T = Signal; // keep track of lowest point in pulse wave
        }
    }

    if (Signal > thresh && Signal > P) { // thresh condition helps avoid noise
        P = Signal; // P is the peak
    } // keep track of highest point in pulse wave

    //  NOW IT'S TIME TO LOOK FOR THE HEART BEAT
    // signal surges up in value every time there is a pulse
    if (N > 250) { // avoid high frequency noise
        if ((Signal > thresh) && (Pulse == false) && (N > (IBI / 5) * 3)) {
            Pulse = true; // set the Pulse flag when we think there is a pulse
            IBI = sampleCounter - lastBeatTime; // measure time between beats in mS
            lastBeatTime = sampleCounter; // keep track of time for next pulse

            if (secondBeat) { // if this is the second beat, if secondBeat == TRUE
                secondBeat = false; // clear secondBeat flag
                int i;
                for (i = 0; i <= 9; i++) { // seed the running total to get a realistic BPM at startup
                    rate[i] = IBI;
                }
            }

            if (firstBeat) { // if it's the first time we found a beat, if firstBeat == TRUE
                firstBeat = false; // clear firstBeat flag
                secondBeat = true; // set the second beat flag
                return; // IBI value is unreliable so discard it
            }

            // keep a running total of the last 10 IBI values
            uint16_t runningTotal = 0; // clear the runningTotal variable
            int i;
            for (i = 0; i <= 8; i++) { // shift data in the rate array
                rate[i] = rate[i + 1]; // and drop the oldest IBI value
                runningTotal += rate[i]; // add up the 9 oldest IBI values
            }

            rate[9] = IBI; // add the latest IBI to the rate array
            runningTotal += rate[9]; // add the latest IBI to runningTotal
            runningTotal /= 10; // average the last 10 IBI values
            BPM = 60000 / runningTotal; // how many beats can fit into a minute? that's BPM!
            QS = true; // set Quantified Self flag
        }
    }

    if (Signal < thresh && Pulse == true) { // when the values are going down, the beat is over
        Pulse = false; // reset the Pulse flag so we can do it again
        amp = P - T; // get amplitude of the pulse wave
        thresh = amp / 2 + T; // set thresh at 50% of the amplitude
        P = thresh; // reset these for next time
        T = thresh;
    }

    if (N > 2500) { // if 2.5 seconds go by without a beat
        thresh = 530; // set thresh default
        P = 512; // set P default
        T = 512; // set T default
        lastBeatTime = sampleCounter; // bring the lastBeatTime up to date
        firstBeat = true; // set these to avoid noise
        secondBeat = false; // when we get the heartbeat back
    }
}

void buzzer_control(bool is_high) {
    if (is_high) {
        // Code to generate a high frequency sound on RB3
        // Example: 4 kHz frequency
        for (int i = 0; i < 100; i++) {
            RB3 = 1;
            __delay_us(1000); // Half period for 4 kHz
            RB3 = 0;
            __delay_us(1000);
        }
    } else {
        // Code to generate a normal frequency sound on RB3
        // Example: 2 kHz frequency
        for (int i = 0; i < 100; i++) {
            RB3 = 1;
            __delay_us(500); // Half period for 2 kHz
            RB3 = 0;
            __delay_us(500);
        }
    }
}
// Function to control the RGB LED
void rgb_led_control(int bpm) {
    if (bpm < 60) {
        // Blue color
        PORTCbits.RC5 = 0; // Red off
        PORTCbits.RC6 = 0; // Green off
        PORTCbits.RC7 = 1; // Blue on
    } else if (bpm >= 60 && bpm <= 100) {
        // Green color
        PORTCbits.RC5 = 0; // Red off
        PORTCbits.RC6 = 1; // Green on
        PORTCbits.RC7 = 0; // Blue off
    } else {
        // Red color
        PORTCbits.RC5 = 1; // Red on
        PORTCbits.RC6 = 0; // Green off
        PORTCbits.RC7 = 0; // Blue off
    }
}
void main() {
    system_init();
    main_state = READ_ADC;
    int bpm_readings[5] = {0}; // Array to store the BPM readings
    int ibi_readings[5] = {0}; // Array to store the IBI readings
    int reading_count = 0; // Counter for BPM and IBI readings
    while (1) {
        switch (main_state) {
            case READ_ADC:
                adc_value = ADC_Read(0);
                if (adc_value > 100) { // If a finger is detected
                    main_state = CALCULATE_HEART_BEAT;
                }
                break;

            case CALCULATE_HEART_BEAT:
                calculate_heart_beat(adc_value);
                main_state = SHOW_HEART_BEAT;
                break;

            case SHOW_HEART_BEAT:
                if (QS == true) { // A Heartbeat Was Found
                    QS = false; // reset the Quantified Self flag for next time

                    lcd_com(0x01); // Clear the LCD
                    __delay_ms(2);
                    lcd_com(0x80); // Move to the first line
                    lcd_puts("Reading..."); // Display reading message
                    __delay_ms(1000); // Short delay to show reading message

                    lcd_com(0x01); // Clear the LCD
                    __delay_ms(2);
                    lcd_com(0x80); // Move to the first line

                    bpm_readings[reading_count] = BPM; // Store the BPM reading
                    ibi_readings[reading_count] = IBI; // Store the IBI reading
                    reading_count++;

                    if (reading_count >= 5) { // If we have 5 readings
                        int total_bpm = 0;
                        int total_ibi = 0;
                        for (int i = 0; i < 5; i++) {
                            total_bpm += bpm_readings[i];
                            total_ibi += ibi_readings[i];
                        }
                        int avg_bpm = total_bpm / 5;
                        int avg_ibi = total_ibi / 5;

                        lcd_puts("BPM: ");
                        lcd_print_number(BPM);

                        lcd_com(0xC0); // Move to the second line
                        lcd_puts("IBI: ");
                        lcd_print_number(IBI);

                        __delay_ms(2000); // Display for 2 seconds

                        lcd_com(0x01); // Clear the LCD
                        __delay_ms(2);
                        lcd_com(0x80); // Move to the first line
                        if (avg_bpm < 60) {
                            lcd_puts("BPM Low");
                        } else if (avg_bpm > 100) {
                            lcd_puts("BPM High");
                        } else {
                            lcd_puts("BPM Normal");
                        }

                        lcd_com(0xC0); // Move to the second line
                        lcd_puts("Avg BPM: ");
                        lcd_print_number(avg_bpm);

                        // Buzzer control
                        if (avg_bpm < 60 || avg_bpm > 100) {
                            buzzer_control(true); // Generate high frequency sound
                        } else {
                            buzzer_control(false); // Generate normal frequency sound
                        }
                        
                        rgb_led_control(avg_bpm);

                        reading_count = 0; // Reset the counter
                    } else {
                        lcd_puts("BPM: ");
                        lcd_print_number(BPM);

                        lcd_com(0xC0); // Move to the second line
                        lcd_puts("IBI: ");
                        lcd_print_number(IBI);

                        // Buzzer control
                        if (BPM < 60 || BPM > 100) {
                            buzzer_control(true); // Generate high frequency sound
                        } else {
                            buzzer_control(false); // Generate normal frequency sound
                        }
                        rgb_led_control(BPM);
                    }
                }
                main_state = IDLE;
                break;

            default:
                break;
        }
    }
}

void system_init(void) {
    TRISB = 0x00; // Set PORTB as output
    lcd_init(); // Initialize the LCD
    __delay_ms(20); // Add delay for LCD initialization
    lcd_com(0x01); // Clear the LCD
    __delay_ms(2);
    lcd_com(0x80); // Move to the first line
    lcd_puts("Wlc to Medifool"); // Display welcome message
    __delay_ms(2000); // Display the message for 2 seconds
    lcd_com(0x01); // Clear the LCD again
    __delay_ms(2);

    TMR0_Initialize(); // Initialize Timer0
    TMR0_StartTimer(); // Start Timer0

    ADC_Init(); // Initialize ADC

    // Initialize buzzer pin
    TRISBbits.TRISB3 = 0; // Set RB3 as output
    RB3 = 0; // Ensure buzzer is off initially
    
    // Initialize RGB LED pins
    TRISCbits.TRISC5 = 0; // Set RC5 as output for Red
    TRISCbits.TRISC6 = 0; // Set RC6 as output for Green
    TRISCbits.TRISC7 = 0; // Set RC7 as output for Blue

    // Ensure RGB LED is off initially
    PORTCbits.RC5 = 0; // Red off
    PORTCbits.RC6 = 0; // Green off
    PORTCbits.RC7 = 0; // Blue off


    // Enable global and peripheral interrupts
    INTERRUPT_GlobalInterruptEnable();
    INTERRUPT_PeripheralInterruptEnable();
}

void timer_isr() {
    main_state = READ_ADC;    
}

void interrupt INTERRUPT_InterruptManager (void)
{
    // interrupt handler
    if(INTCONbits.TMR0IE == 1 && INTCONbits.TMR0IF == 1)
    {
        TMR0_ISR();
    }
}
