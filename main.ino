// This file lists the code for generating a basic PWM on Timer 0 and 1
// Filename: Timer1.ino
// Date updated: Spring-2016
// Version 1.0

#include <avr/io.h>
#include <avr/io.h>
#include "constants.h"

// ======================================================================================================
// Initialize
// ======================================================================================================
void setup() {
  // ------------------------------------------------------------------------------------------------------
  // Basic steps
  // ------------------------------------------------------------------------------------------------------
  cli();                                       // Disable all interrupts

  GTCCR |= (1 << TSM) | (1 << PSRASY) | (1 << PSRSYNC); // Halt all Timers

  //  ------------------------------------------------------------------------------------------------------
  //   Initial values
  //  ------------------------------------------------------------------------------------------------------
  Run_state = 0;
  runDC = 0;

  // --------------------------------------------------------------------------------------------------------
  // Timer 0 settings
  // --------------------------------------------------------------------------------------------------------
  TCCR0A = 0;                           // Reset Timer 0 Control Register A
  TCCR0B = 0;                           // Reset Timer 0 Control Register B
  TCCR0A |= (1 << COM0A1) |             // Clear OC0A on compare match up counting, Set on down counting
            (0 << COM0A0) |
            (1 << COM0B1) |             // Clear OC0B on compare match up counting, Set on down counting
            (0 << COM0B0) |
            (1 << WGM00)  |             // Fast PWM mode, 8-bit TOP = 0xFF
            (1 << WGM01)  ;
  TCCR0B |= (0 << WGM02)  |
            (0 << CS02)   |             // Prescalar N = 8, clock frequency = 16MHz
            (1 << CS01)   |
            (0 << CS00)   ;

  OCR0A = DUTYRATIO;                         // Default Timer 0A duty cycle
  OCR0B = DUTYRATIO;                         // Default Timer 0B duty cycle

  TIMSK0 = 0;                           // Reset Timer 0 interrupt mask register
  TIMSK0 |= (1 << OCIE0A) ;             // Timer 0 overflow interrupt TOV0 bit set when TCNT0 = BOTTOM
  TIFR0 = 0;                            // Reset Timer 0 interrupt flag register
  TCNT0 = 0x00;                         // Set Timer 0 counter to 0
  // --------------------------------------------------------------------------------------------------------

  // --------------------------------------------------------------------------------------------------------
  // Timer 1 settings
  // --------------------------------------------------------------------------------------------------------
  TCCR1A = 0;                           // Reset Timer 1 Control Register A
  TCCR1B = 0;                           // Reset Timer 1 Control Register B
  TCCR1A |= (1 << COM1A1) |             // Clear OC1A on compare match up counting, Set on down counting
            (0 << COM1A0) |
            (1 << COM1B1) |             // Set OC1B on compare match up counting, Clear on down counting
            (1 << COM1B0) |
            (0 << WGM10)  |             // Phase & frequency correct mode, 8-bit TOP = ICR1
            (0 << WGM11)  ;
  TCCR1B |= (0 << WGM12)  |
            (1 << WGM13)  |
            (0 << CS12)   |             // Prescalar N = 1, clock frequency = 16MHz
            (0 << CS11)   |
            (1 << CS10)   ;

  ICR1 = PERIOD;
  OCR1A = DUTYRATIO;                    // Default Timer 1A duty cycle
  OCR1B = DUTYRATIO;                         // Default Timer 1B duty cycle

  TIMSK1 = 0;                           // Reset Timer 1 interrupt mask register
  TIFR1 = 0;                            // Reset Timer 1 interrupt flag register
  TCNT1 = 0x00;                         // Set Timer 1 counter to 0
  // --------------------------------------------------------------------------------------------------------

  // --------------------------------------------------------------------------------------------------------
  // IO settings
  // --------------------------------------------------------------------------------------------------------
  SREG = 0x00;                // Reset AVR status
  SREG |= (1 << 7) ;          // Enable global Interrupt

  GTCCR = 0;                  // Release all timers

  MCUCR = 0;                  // Reset MCU control register
  MCUCR |= (1 << PUD);        // Disable pull-up for all IO pins

  DDRB = 0;                   // Reset data direction register Port B, all input
  DDRD = 0;                   // Reset data direction register Port D, all input
  DDRC = 0;                   // Reset data direction register Port C, all input

  // Digital pins
  DDRD  |= (1 << DDD2) |      // Green LED
           (1 << DDD4) |      // Red LED
           (1 << DDD6) |      // Timer 0A Pin set as output
           (1 << DDD5) ;      // Timer 0B Pin set as output
  DDRB  |= (1 << DDB1) |      // Timer 1A Pin set as output
           (1 << DDB2) ;      // Timer 1B Pin set as output

  PORTD |= (1 << PORTD4);     // Red LED high (off)
  PORTD |= (1 << PORTD2);     // Green LED high (off)

  sei();                      // Enable all interrupts
}

// ======================================================================================================
// Computing clock: state machine
// ======================================================================================================
ISR(TIMER0_COMPA_vect) {
  PORTD &= ~(1 << PORTD2); // Computational load indicator: low

  switchstate = PINB;
  //   PWM off by 180V_SW
  if (CHECKBIT(switchstate, 0)) {// If 180V_SW high, stop pwm
    PORTD |= (1 << PORTD4);      // Red Light on
    DDRB &= ~(1 << DDB1) &       // PWM1A Output 0 ("180-AC" port)
            ~(1 << DDB2) ;       // PWM1B Output 0 ("180-AC" port)
    Run_state = 0;
    runDC = 0;
  } else if (Run_state == 0) {   // If 180V_SW low and idle status, start pwm
    PORTD &= ~(1 << PORTD4);     // Red Light off
    DDRB  |= (1 << DDB1) |       // Timer 1A Pin set as output
             (1 << DDB2) ;       // Timer 1B Pin set as output
    Run_state = 1;
    runDC = 1;
  }

  PORTD |= (1 << PORTD2); // Computational load indicator: high
}

// ======================================================================================================
// Looping
// ======================================================================================================
void loop() {
  asm ("nop\n\t");

}

/*
  Digital Pin 12 (PB4) - Connected to 120_SW
  Digital Pin 08 (PB0) - Connected to 180_SW
  Digital Pin 02 (PD2) - Connected to Green LED D4 on PCB
  Digital Pin 04 (PD4) - Connected to Red LED D5 on PCB
*/
