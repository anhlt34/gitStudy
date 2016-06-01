// Author:			Le Tuan Anh
// Date:				121/04/2015
// Description:	
//		1.	Create a Keil project for FRD KL46Z256 board
//		2.	Using SysTick to implement the code to blink two LEDs as the following:  RED LED blinks at 1Hz; Green LED blink at 1 Hz.
//		3.	Press button 1 to increase Green LED frequency twice; and Press button 2 to decrease Green LED frequency twice.
//		4.	SysTick & GPIO modules are configured in interrupt mode.

#include "MKL46Z4.h"                    // Device header

// Config PORT
uint32_t RED_LED = 1ul << 29;
uint32_t GREEN_LED = 1ul << 5;
uint32_t SW1 = 1ul << 3;
uint32_t SW3 = 1ul << 12;

// Bien tan so nhay 2 led
volatile uint32_t RED_F = 1000; // tranh loi khi optimization. Do co ISR lam thay doi bien toan cuc
volatile uint32_t GREEN_F = 1000;

// COUNTER TICK
volatile uint32_t counter = 0;

// x, y la input tu sw1, sw3
uint32_t x,y;

extern uint32_t SystemCoreClock;
void SystemCoreClockUpdate (void);

void SysTick_Handler(void){
	
	counter++;							// Counts 1ms timeTicks
	
	// thiet lap tan so nhay
	if((counter % RED_F) == 0)
		PTE->PTOR |= RED_LED;	// dao trang thai RED led
	
	if((counter % GREEN_F) == 0)
		PTD->PTOR |= GREEN_LED;// dao trang thai GREEN led
}

void init_RED_LED(){
	SIM->SCGC5 |= SIM_SCGC5_PORTE_MASK;	// ENABLE CLOCK
	PORTE->PCR[29] = PORT_PCR_MUX(1UL); // E29 LA GPIO
	PTE->PDDR |= RED_LED; 							// E29  = OUTPUT
}

void init_GREEN_LED(){
	SIM->SCGC5 |= SIM_SCGC5_PORTD_MASK;
	PORTD->PCR[5] = PORT_PCR_MUX(1UL);
	PTD->PDDR |= GREEN_LED;
	
}

void init_SW1(){
	SIM->SCGC5 |= SIM_SCGC5_PORTC_MASK;
	PORTC->PCR[3] |=  PORT_PCR_MUX(1UL) 
									| PORT_PCR_PS_MASK
									| PORT_PCR_PE_MASK;
	PORTC->PCR[3] |= PORT_PCR_IRQC(0xA)	;// GPIO + pullUp enable+ pull enable + Interrupt on falling edge.
	PTC->PDDR &= ~SW1; // PTC3  = input;
	
}
void init_SW3(){
	SIM->SCGC5 |= SIM_SCGC5_PORTC_MASK;
	PORTC->PCR[12] = PORT_PCR_MUX(1UL) + PORT_PCR_PS_MASK + PORT_PCR_PE_MASK; // GPIO + pullUp enable+ pull enable
	PORTC->PCR[12] |= PORT_PCR_IRQC(0xA)	;
	PTC->PDDR &= ~SW1; // PTC12  = input;
}

void int_interrupt_button(){
	NVIC_ClearPendingIRQ(PORTC_PORTD_IRQn);
	NVIC_EnableIRQ(PORTC_PORTD_IRQn);
	NVIC_SetPriority(PORTC_PORTD_IRQn, 0);
}

void PORTC_PORTD_IRQHandler(){
	
	if(PORTC->PCR[3] & PORT_PCR_ISF_MASK){
		GREEN_F = GREEN_F * 2; // Tan so cua RED led tang 2 lan
		PORTC->PCR[3] |= PORT_PCR_ISF_MASK;	// xoa co ngat
	}
	
	if(PORTC->PCR[12] & PORT_PCR_ISF_MASK){
		GREEN_F = GREEN_F / 2; // Tan so cua GREEN led giam 2 lan
		PORTC->PCR[12] |= PORT_PCR_ISF_MASK;	//
		//PORTC->PCR[12] &= ~PORT_PCR_ISF_MASK;	//
	}
}


// MAIN
int main(){
	
	uint32_t ticks = SystemCoreClock/1000;
	SysTick_Config(ticks);
	
	init_RED_LED();
	init_GREEN_LED();
	init_SW1();
	init_SW3();
	int_interrupt_button();
	while(1){
//		x = PTC->PDIR & SW3; // x = input
//		y = PTC->PDIR & SW1; // y = input
//		if(x != SW3)// if press sw3
//		{
//			while(x != SW3)
//			{
//				x = PTC->PDIR & SW3; // chong doi phim khi an va giu
//			}

//			GREEN_F = GREEN_F / 2; // Tan so cua GREEN led giam 2 lan
//		}
//		
//		if(y != SW1)// if press sw1
//		{
//			while(y != SW1)
//			{
//				y = PTC->PDIR & SW1; // chong doi phim khi an va giu
//			}
//			GREEN_F = GREEN_F * 2; // Tan so cua RED led tang 2 lan
//		}
	}
}
