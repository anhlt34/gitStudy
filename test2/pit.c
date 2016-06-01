/******************************************************************************
*
* Freescale Semiconductor Inc.
* (c) Copyright 2004-2005 Freescale Semiconductor, Inc.
* (c) Copyright 2001-2004 Motorola, Inc.
* ALL RIGHTS RESERVED.
*
***************************************************************************//*!
*
* @file main.c
*
* @author b01252
*
* @version 1.0
*
* @date Mar-10-2004
*
* @brief Brief description of the file
*
*******************************************************************************
*
*  Provides initialization and interrupt service for PIT 
*   
******************************************************************************/
#include "MKL46Z4.h"                    // Device header

int ti_task=200;

/**   PIT_init
 * \brief    Initialize Periodic interrupt timer,
 * \brief    PIT1 is used for tone/buzzer time control
 * \author   b01252
 * \param    none
 * \return   none
 */  
void Pit_init(void)
{
    SIM->SCGC6 |= SIM_SCGC6_PIT_MASK; // enable PIT module
    
    /* Enable PIT Interrupt in NVIC*/   
    /* Set the ICPR and ISER registers accordingly */
    NVIC->ICPR[0] |= 1 << (22%32);
    NVIC->ISER[0] |= 1 << (22%32);
       
    PIT->MCR = 0x00;  // MDIS = 0  enables timer
    PIT->CHANNEL[0].TCTRL = 0x00; // disable PIT0
    //PIT->TCTRL1 = 0x00; // disable PIT0
    PIT->CHANNEL[0].LDVAL = 6000; // 
    PIT->CHANNEL[0].TCTRL = PIT_TCTRL_TIE_MASK; // enable PIT0 and interrupt
    PIT->CHANNEL[0].TFLG = 0x01; // clear flag
    PIT->CHANNEL[0].TCTRL |= PIT_TCTRL_TEN_MASK;
}

/**   PIT_init
 * \brief    Periodic interrupt Timer 1.  Interrupt service
 * \brief    PIT1 is used for tone/buzzer time control
 * \author   b01252
 * \param    none
 * \return   none
 */  

void PIT_IRQHandler(void)
{  
    PIT->CHANNEL[0].TFLG = 0x01; // clear flag

    if (ti_task) ti_task--;
    if (!ti_task)
    {
			  ti_task=200;
        PTE->PTOR |= GPIO_PTOR_PTTO(1<<29);// toggle PTE29
    }
}
