// This file is for ECE 512 Fall-2019 LAB 9, 10 to generate SPWM with timer 2
 // Date updated: Fall-2019
 // Version 1.0

#include <avr/io.h>
#include <avr/io.h>
#include "constants.h"
#include "sinetable256.h"    // sinetable with 256 entries 

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
  runAC = 0;
  climb = 1;
  dutyratio = DUTYRATIO;
  PointerSPWM = 0;//////////////////////////////////////////////////////////////

  // --------------------------------------------------------------------------------------------------------
  // Timer 0 settings: There are no changes in this section
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
  // Timer 1 settings //No changes in this part, dc/dc converter PWM 
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
  OCR1A = DUTYRATIO-DEADTIME;                    // Default Timer 1A duty cycle
  OCR1B = DUTYRATIO;           // Default Timer 1B duty cycle

  TIMSK1 = 0;                           // Reset Timer 1 interrupt mask register
  TIFR1 = 0;                            // Reset Timer 1 interrupt flag register
  TCNT1 = 0x00;                         // Set Timer 1 counter to 0
  // --------------------------------------------------------------------------------------------------------

  // --------------------------------------------------------------------------------------------------------
  // Timer 2 settings (((((New part for 60 Hz SPWM generation))))
  // --------------------------------------------------------------------------------------------------------
  TCCR2A = 0;                           // Reset Timer 2 Control Register A
  TCCR2B = 0;                           // Reset Timer 2 Control Register B
  TCCR2A |= (1 << COM1A1) |             // Clear OC2A on compare match up counting, Set on down counting
            (0 << COM1A0) |
            (1 << COM1B1) |             // Set OC2B on compare match up counting, Clear on down counting
            (1 << COM1B0) |
            (1 << WGM10)  |             // Phase correct mode, 8-bit TOP = 0xFF
            (0 << WGM11)  ;
  TCCR2B |= (0 << WGM12)  |
            (0 << CS12)   |             // Prescalar N = 1, clock frequency = 16MHz
            (0 << CS11)   |
            (1 << CS10)   ;

  OCR2A = DUTYRATIO - DEADTIME;         // Default Timer 2A duty cycle
  OCR2B = 0xFF - OCR1A;                 // Default Timer 2B duty cycle

  TIMSK2 = 0;                           // Reset Timer 2 interrupt mask register
  TIFR2 = 0;                            // Reset Timer 2 interrupt flag register
  TCNT2 = 0x00;                         // Set Timer 2 counter to 0
  // --------------------------------------------------------------------------------------------------------
  
  ASSR = 0; // Reset Async status register, TIMER2 clk = CPU clk
  // This ia new part (((())))

  
  // --------------------------------------------------------------------------------------------------------
  // IO settings (((No change))))
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
  
  switchstate[4] = switchstate[3];
  switchstate[3] = switchstate[2];
  switchstate[2] = switchstate[1];
  switchstate[1] = switchstate[0];
  switchstate[0] = PINB;

  
  //   PWM off by 180V_SW
  if (CHECKBIT(switchstate[0], 0)) {// If 180V_SW high, stop pwm
    PORTD |= (1 << PORTD4);      // Red Light off
    DDRB &= ~(1 << DDB3) ;      // PWM2A Output 0 ("test" port)
    DDRD &= ~(1 << DDD3) ;      // PWM2B Output 0 ("test" port)
    Run_state = 0;
    runDC = 0; // runAC = 0 means that 180V_SW is turned off
    runAC = 0;
   PointerSPWM = 0;
  } else if (Run_state == 0) {   // If 180V_SW low and idle status, start pwm
    PORTD &= ~(1 << PORTD4);     // Red Light on
    DDRB  |= (1 << DDB3) ;       // Timer 2A Pin set as output
    DDRD  |= (1 << DDD3) ;       // Timer 2B Pin set as output
    Run_state = 1;
    runDC = 1; 
    runAC = 0; // runDC = 1 means that 180V_SW is turned on
  }

// PWM tuning by 120V_SW Now it is for the dc/dc converter
  if (runDC){
    unsigned int status_40 = CHECKBIT(switchstate[0], 4);
    unsigned int status_41 = CHECKBIT(switchstate[1], 4);
    unsigned int status_42 = CHECKBIT(switchstate[2], 4);
    unsigned int status_43 = CHECKBIT(switchstate[3], 4);
    unsigned int status_44 = CHECKBIT(switchstate[4], 4);
    if ((status_40 ^ status_41) && (status_41==status_42) && (status_41==status_43) && (status_41==status_44)){
      if (dutyratio + CLIMB_STEP > DUTYRATIO_MAX){
        climb = DECREASING;
      }
      else if (dutyratio - CLIMB_STEP < DUTYRATIO_MIN){
        climb = INCREASING;
      }      
      dutyratio += climb*CLIMB_STEP;
      OCR1A = dutyratio-DEADTIME;
      OCR1B = dutyratio;      
      
    }
  }

    
  //   Sine Wave update
  if (runAC == 1) {
    outSPWM = 0.9*(sinetable256[PointerSPWM])+(1-0.9)*AMPSPWM;
    if (outSPWM > MAXSPWM) {
      outSPWM = MAXSPWM;
    }
    else if (outSPWM < MINSPWM) {
      outSPWM = MINSPWM;
    }
    OCR2A = outSPWM - DEADTIME;
    OCR2B = outSPWM + DEADTIME;
    PointerSPWM += POINTERSTEP;
    if (PointerSPWM > LENGTHSPWM) {
      PointerSPWM = 0;
    }
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
