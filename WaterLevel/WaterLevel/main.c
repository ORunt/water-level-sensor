/*
 * WaterLevel.c
 *
 * Created: 2024/04/11 21:09:11
 * Author : Cameron
 */ 

#include "HC_SR04_AT85.h"

//#define CALIB_CLK
//#define TOGGLE

#define TOG_PIN     PB2
#define PIN_FULL    PB2
#define PIN_EMPTY   PB1

#define FULL_THRESH     200     // 0.2m
#define EMPTY_THRESH    1400    // 1.4m

#define ACTIVE_LOW

static void GPIOinit(void)
{
    DDRB |= (1<<PIN_FULL);          // Set output _BV(pin)
    DDRB |= (1<<PIN_EMPTY);         // Set output
    //DDRB |= (1<<PB4);         // Set output
    #ifdef ACTIVE_LOW
    PORTB = PINB | (1<<PIN_FULL);  // Set pin
    PORTB = PINB | (1<<PIN_EMPTY); // Set pin
    #else
    PORTB = PINB & ~(1<<PIN_FULL);  // Reset pin
    PORTB = PINB & ~(1<<PIN_EMPTY); // Reset pin
    #endif
    //PORTB = PINB & ~(1<<PB4); // Reset pin
}

#if defined(CALIB_CLK)

void main(void)
{
    calibrateClockTest();
    while(1);
}

#elif defined(TOGGLE)

void main(void)
{
    uint8_t toggle = 0;
    
    overclock();
    
    DDRB |= (1<<TOG_PIN);
    PORTB = PINB & ~(1<<TOG_PIN);  // Reset pin
    
    while(1)
    {
        _delay_ms(1000); // 5 seconds
        if(toggle){
            PORTB |= (1<<TOG_PIN);
            toggle = 0;
        } else {
            PORTB = PINB & ~(1<<TOG_PIN);
            toggle = 1;
        }
    }
}

#else

int main(void)
{
    uint32_t current_level_mm = 0;
    
    overclock();
    
    SONARinit();
    GPIOinit();
    
    while (1) 
    {
        current_level_mm = SONARavg();
        /*plotValue((current_level_mm >> 24) & 0xFF);
        plotValue((current_level_mm >> 16) & 0xFF);
        plotValue((current_level_mm >> 8) & 0xFF);
        plotValue((current_level_mm >> 0) & 0xFF);*/
        
        if(current_level_mm < FULL_THRESH){
            #ifdef ACTIVE_LOW
            PORTB = PINB & ~(1<<PIN_FULL);
            PORTB |= (1<<PIN_EMPTY);
            #else
            PORTB = PINB & ~(1<<PIN_EMPTY);
            PORTB |= (1<<PIN_FULL);
            #endif
        }
        else if(current_level_mm > EMPTY_THRESH){
            #ifdef ACTIVE_LOW
            PORTB = PINB & ~(1<<PIN_EMPTY);
            PORTB |= (1<<PIN_FULL);
            #else
            PORTB = PINB & ~(1<<PIN_FULL);
            PORTB |= (1<<PIN_EMPTY);
            #endif
        }
        else{
            #ifdef ACTIVE_LOW
            PORTB = PINB | ((1<<PIN_FULL) | (1<<PIN_EMPTY));
            #else
            PORTB = PINB & ~((1<<PIN_FULL) | (1<<PIN_EMPTY));
            #endif
        }
        _delay_ms(1000); // 5 seconds
    }
}

#endif