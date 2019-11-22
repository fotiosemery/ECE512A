 // This file is for ECE 512 Fall-2019 LAB 9, 10 to generate SPWM with timer 2
 // Date updated: Fall-2019
 // Version 1.0

// Interrupt related 
#define SETBIT(ADDRESS,BIT) (ADDRESS |= (1<<BIT))
#define CLEARBIT(ADDRESS,BIT) (ADDRESS &= ~(1<<BIT))
#define CHECKBIT(ADDRESS,BIT) (ADDRESS & (1<<BIT))
#define TOGGLEBIT(ADDRESS,BIT) (ADDRESS ^= (1<<BIT))

// PWM related
unsigned int runDC;
#define DEADTIME    8
#define PERIOD      400   // 25kHz:320; 20kHz:400
#define DUTYRATIO   176
#define MAXTIMECOUNTDC 2000
unsigned int dutyratio;       // This variable can be changed to modulation index
#define DUTYRATIO_MAX  180      // This constant can be changed to MODULATION_INDEX_MAX
#define DUTYRATIO_MIN  20      // this constrant can be changed to MODULATION_INDEX_MIN

//Sine Wave related     this is for SPWM at timer2
unsigned int runAC;
#define MAXSPWM     0xFF - DEADTIME
#define MINSPWM     DEADTIME    // 8
#define AMPSPWM     128         // dc bias point of timer2 (256/2)
#define LENGTHSPWM  255         // (total sine look up table entries) -1
unsigned int PointerSPWM;
int outSPWM;
#define MAXTIMECOUNTAC 2
#define POINTERSTEP 2           // This constant changes from 4 to 2 when the sinetable only has 256 entries
                                // while maintaining 60Hz output voltage. Please refer to LAB 9, 9.1 Objective for more information
                                // 7800*POINTERSTEP/Sinetable entries = 60.9 Hz
// State machine and protection related
#define TRUE 1
#define FALSE 0
unsigned char switchstate[5];
unsigned int Run_state;
int climb;
#define INCREASING 1
#define DECREASING -1
#define CLIMB_STEP 4
