 // This file is associated with the file that lists the code for generating a PWM on Timer 1
 // Filename: Timer1.ino
 // Date updated: Spring-2016
 // Version 1.0

// Interrupt related 
#define SETBIT(ADDRESS,BIT) (ADDRESS |= (1<<BIT))
#define CLEARBIT(ADDRESS,BIT) (ADDRESS &= ~(1<<BIT))
#define CHECKBIT(ADDRESS,BIT) (ADDRESS & (1<<BIT))
#define TOGGLEBIT(ADDRESS,BIT) (ADDRESS ^= (1<<BIT))

//PWM related
unsigned int runDC;
#define DEADTIME    8
#define PERIOD      160   // 25kHz:320; 20kHz:400
#define DUTYRATIO   80
#define MAXTIMECOUNTDC 2000

// State machine and protection related
#define TRUE 1
#define FALSE 0
unsigned char switchstate;
unsigned int Run_state;
