// TableTrafficLight.c edX lab 10, EE319K Lab 5
// Runs on LM4F120 or TM4C123
// Index implementation of a Moore finite state machine to operate a traffic light.  
// Daniel Valvano, Jonathan Valvano
// Feb 27, 2017

/* 
 Copyright 2018 by Jonathan W. Valvano, valvano@mail.utexas.edu
    You may use, edit, run or distribute this file
    as long as the above copyright notice remains
 THIS SOFTWARE IS PROVIDED "AS IS".  NO WARRANTIES, WHETHER EXPRESS, IMPLIED
 OR STATUTORY, INCLUDING, BUT NOT LIMITED TO, IMPLIED WARRANTIES OF
 MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE APPLY TO THIS SOFTWARE.
 VALVANO SHALL NOT, IN ANY CIRCUMSTANCES, BE LIABLE FOR SPECIAL, INCIDENTAL,
 OR CONSEQUENTIAL DAMAGES, FOR ANY REASON WHATSOEVER.
 For more information about my classes, my research, and my books, see
 http://users.ece.utexas.edu/~valvano/
 */

// See TExaS.h for other possible hardware connections that can be simulated
// north/south red light connected to PE5
// north/south yellow light connected to PE4
// north/south green light connected to PE3
// east/west facing red light connected to PE2
// east/west facing yellow light connected to PE1
// east/west facing green light connected to PE0
// pedestrian detector connected to PA4 (1=pedestrian present)
// east/west car detector connected to PA3 (1=car present)
// north/south car detector connected to PA2 (1=car present)
// "walk" light connected to PF3,2,1 (built-in white LED)
// "don't walk" light connected to PF1 (built-in red LED)
#include <stdint.h>
#include "tm4c123gh6pm.h"
#include "SysTick.h"
#include "TExaS.h"
#include <stdio.h>


// Declare your FSM linked structure here
	struct State{
unsigned long Out;
unsigned long Time;
unsigned long Next[14];};
typedef const struct State STyp;
#define goS 0
#define slowS 1
#define waitS 2
#define goW 3
#define slowW 4
#define waitW 5
#define walk 6
#define flashOn1 7
#define flashOff1 8
#define flashOn2 9
#define flashOff2 10
#define flashOn3 11
#define flashOff3 12
#define noWalk 13

STyp FSM[14]={
	{0x4C, 36, {goS, goS, slowS, slowS, slowS, slowS, slowS, slowS, slowS, slowS, slowS, slowS, slowS, slowS}},
	{0x54, 36, {waitS, waitS, waitS, waitS, waitS, waitS, waitS, waitS}},
	{0x64, 36, {goW, goW, goW, goW, walk, walk, goW, goW}},
	{0x61, 36, {goW, slowW, goW, slowW, slowW, slowW, slowW, slowW}},
	{0x62, 36, {waitW, waitW, waitW, waitW, waitW, waitW, waitW, waitW, }},
	{0x64, 36, {walk, goS, walk, goS, walk, walk, walk, walk,}},
	{0xA4, 36, {walk, flashOn1, flashOn1, flashOn1, walk, flashOn1, flashOn1, flashOn1}},
	{0x64, 18, {flashOff1, flashOff1, flashOff1, flashOff1, flashOff1, flashOff1, flashOff1, flashOff1}},
	{0x24, 18, {flashOn2, flashOn2, flashOn2, flashOn2, flashOn2, flashOn2, flashOn2, flashOn2}},
	{0x64, 18, {flashOff2, flashOff2, flashOff2, flashOff2, flashOff2, flashOff2, flashOff2, flashOff2, }},
	{0x24, 18, {flashOn3, flashOn3, flashOn3, flashOn3, flashOn3, flashOn3, flashOn3, flashOn3}},
	{0x64, 18, {flashOff3, flashOff3, flashOff3, flashOff3, flashOff3, flashOff3, flashOff3, flashOff3, }},
	{0x24, 36, {noWalk, noWalk, noWalk, noWalk, noWalk, noWalk, noWalk, noWalk}},
	{0x64, 36, {goS, goS, goW, goS, goS, goS, goW, goS}}

};
unsigned long S;
unsigned long Input;
unsigned long lightData;
unsigned long lightData2;

void EnableInterrupts(void);
void PortInit(void){ volatile unsigned long delay;
	//SYSCTL_RCGCGPIO_R |= 0x00000031;  // 1) activate clock for Port A,E,F
  //delay = SYSCTL_RCGCGPIO_R;        // allow time for clock to start
	SYSCTL_RCGC2_R |= 0x31;
	delay = SYSCTL_RCGC2_R;
  GPIO_PORTF_LOCK_R = 0x4C4F434B;   // 2) unlock GPIO Port F
	GPIO_PORTF_CR_R = 0x1F;						//unlock ports
  GPIO_PORTF_AMSEL_R &= ~0x0E;        // 3) disable analog on PF3-1
  GPIO_PORTF_PCTL_R &= ~0x0E;   // 4) PCTL GPIO on PF3-1
  GPIO_PORTF_DIR_R |= 0x0E;          // 5) PF3-1 out
  GPIO_PORTF_AFSEL_R &= ~0x0E;        // 6) disable alt funct on PF3-1
  GPIO_PORTF_DEN_R |= 0x0E;          // 7) enable digital I/O on PF3-1
	GPIO_PORTA_AMSEL_R &= ~0x1C;        // 8) disable analog on PA4-2
  GPIO_PORTA_DIR_R &= ~0x1C;          // 9) PA4-2 input
  GPIO_PORTA_AFSEL_R &= ~0x1C;        // 10) disable alt funct on PA4-2
  GPIO_PORTA_DEN_R |= 0x1C;          // 11) enable digital I/O on PA4-2
	GPIO_PORTE_AMSEL_R &= ~0x3F;        // 12) disable analog on PE5-0
  GPIO_PORTE_DIR_R |= 0x3F;          // 13) PE5-0 output
  GPIO_PORTE_AFSEL_R &= ~0x3F;        // 14) disable alt funct on PE5-0
  GPIO_PORTE_DEN_R |= 0x3F;          // 15) enable digital I/O on PE5-0
}

int main(void){
  TExaS_Init(SW_PIN_PA432, LED_PIN_PE543210); // activate traffic simulation and set system clock to 80 MHz
  SysTick_Init();     
  // Initialize ports and FSM you write this as part of Lab 5
	PortInit();
  EnableInterrupts(); // TExaS uses interrupts
	S = goS;

  while(1){
 // FSM Engine
 // you write this as part of Lab 5
		GPIO_PORTE_DATA_R= FSM[S].Out&0x3F;
		lightData=FSM[S].Out&0xE0;
		lightData= lightData >>5;
		lightData2= lightData&0x04;
		lightData2=lightData2<<1;
		lightData|= lightData2;
		lightData2= lightData&0x04;
		lightData2=lightData2>>1;
		lightData|= lightData2;		
		GPIO_PORTF_DATA_R= lightData;
		SysTick_Wait10ms(FSM[S].Time);
		Input= GPIO_PORTA_DATA_R&0x1C;
		Input= Input >> 2;
		//printf("The integer is %d\n", FSM[S].Time);
		S= FSM[S].Next[Input];
		

		
		
    
  }
}

