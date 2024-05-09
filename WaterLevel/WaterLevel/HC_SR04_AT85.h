/*
 * HC-SR04_AT85.h
 *
 * Created: 2014-02-15 23:08:56
 *  Author: Wim
 */ 


#ifndef HC_SR04_AT85_H_
#define HC_SR04_AT85_H_

#ifndef F_CPU
#define F_CPU 1000000UL
#endif

#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>

#define TRIGPRT PORTB
#define TRIGDDR DDRB
#define TRIGPIN PB3
#define ECHOPRT PINB
#define ECHODDR DDRB
#define ECHOPIN PB4
#define ECHOPCINT PCINT4

#define overclock() OSCCAL = 110 // 110(v1 water level board); 81(dev pcb)

void calibrateClockTest(void);
void plotValue(uint8_t val);
void SONARinit();
uint32_t SONARread();
uint32_t SONARavg();



#endif /* HC_SR04_AT85_H_ */
