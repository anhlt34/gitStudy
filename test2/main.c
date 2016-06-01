#include "MKL46Z4.h"                    // Device header
#include "uart.h"
#include "stdio.h"

/* Symbols representing MCG modes */
#define MCG_MODE_FEI                    0x00U
#define MCG_MODE_FBI                    0x01U
#define MCG_MODE_BLPI                   0x02U
#define MCG_MODE_BLPE                   0x03U
#define MCG_MODE_FEE                    0x04U
#define MCG_MODE_FBE                    0x05U
#define MCG_MODE_PBE                    0x06U
#define MCG_MODE_PEE                    0x07U

static const uint8_t MCGTransitionMatrix[8][8] = {
/* This matrix defines which mode is next in the MCG Mode state diagram in transitioning from the
   current mode to a target mode
      FEI            FBI              BLPI            BLPE         FEE             FBE            PBE            PEE
        0              1                2              3            4               5              6              7
      */                                                       
{  MCG_MODE_FEI,  MCG_MODE_FBI,  MCG_MODE_FBI,  MCG_MODE_FBE,  MCG_MODE_FEE, MCG_MODE_FBE,  MCG_MODE_FBE,  MCG_MODE_FBE}, /* FEI */
{  MCG_MODE_FEI,  MCG_MODE_FBI,  MCG_MODE_BLPI,  MCG_MODE_FBE, MCG_MODE_FEE,  MCG_MODE_FBE,  MCG_MODE_FBE,  MCG_MODE_FBE},/* FBI */
{  MCG_MODE_FBI,  MCG_MODE_FBI,  MCG_MODE_BLPI,  MCG_MODE_FBE, MCG_MODE_FBI,  MCG_MODE_FBI,  MCG_MODE_FBE,  MCG_MODE_FBI},/* BLPI */
{  MCG_MODE_FBE,  MCG_MODE_FBE,  MCG_MODE_FBI,  MCG_MODE_BLPE, MCG_MODE_FBE,  MCG_MODE_FBE,  MCG_MODE_PBE,  MCG_MODE_PBE},/* BLPE */
{  MCG_MODE_FEI,  MCG_MODE_FBI,  MCG_MODE_FBI,   MCG_MODE_FBE, MCG_MODE_FEE,  MCG_MODE_FBE,  MCG_MODE_FBE,  MCG_MODE_FBE},/* FEE */
{  MCG_MODE_FEI,  MCG_MODE_FBI,  MCG_MODE_FBI,  MCG_MODE_BLPE, MCG_MODE_FEE,  MCG_MODE_FBE,  MCG_MODE_PBE,  MCG_MODE_PBE},/* FBE */
{  MCG_MODE_FBE,  MCG_MODE_FBE,  MCG_MODE_FBI,  MCG_MODE_BLPE, MCG_MODE_FBE,  MCG_MODE_FBE,  MCG_MODE_PBE,  MCG_MODE_PEE},/* PBE */
{  MCG_MODE_FBE,  MCG_MODE_FBE,  MCG_MODE_FBI,  MCG_MODE_PBE,  MCG_MODE_FBE,  MCG_MODE_PBE,  MCG_MODE_PBE,  MCG_MODE_PEE} /* PEE */
};

static const uint8_t * CurMode[8] = 
{
    "MCG_MODE_FEI",
    "MCG_MODE_FBI",
    "MCG_MODE_BLPI",
    "MCG_MODE_BLPE",
	  "MCG_MODE_FEE",
    "MCG_MODE_FBE",
    "MCG_MODE_PBE",
    "MCG_MODE_PEE"
};

uint8_t Cpu_GetCurrentMCGMode(void);
static void Cpu_SetMCG(uint8_t TargetMCGMode);
void MCG_FEI(void);
void MCG_FEE(void);
void MCG_FBI(void);
void MCG_BLPE(void);
void MCG_BLPI(void);
void MCG_FBE(void);
void MCG_PBE(void);
void MCG_PEE(void);
extern void Pit_init(void);
extern void PIT_IRQHandler(void);

void SystemInit()
{
    /* Disable the WDOG module */
    SIM->COPC = (uint32_t)0x00u;

    MCG->SC &= ~MCG_SC_FCRDIV_MASK; // Divide Factor is 1
    MCG->C2 |= MCG_C2_IRCS_MASK; // Fast internal reference clock selected
    MCG->C1 |= MCG_C1_IRCLKEN_MASK;// Enable MCGIRCLK for uart0
}

int main(void)
{
    // set a safe value for divider
    SIM->CLKDIV1 |= SIM_CLKDIV1_OUTDIV4_MASK | SIM_CLKDIV1_OUTDIV1_MASK;
    
    MCG_FEI();

    SIM->CLKDIV1 = SIM_CLKDIV1_OUTDIV1(1 - 1) |  // core/system clock = MCGOUTCLK / 1
                   SIM_CLKDIV1_OUTDIV4(2 - 1);   // flash/bus clock = core/system / 2

    SIM->SCGC5 |= SIM_SCGC5_PORTA_MASK;
    SIM->SOPT2 &= ~SIM_SOPT2_PLLFLLSEL_MASK; // MCGFLLCLK is selected for UART0 clock source 
    SIM->SOPT2 |= SIM_SOPT2_UART0SRC(3);

    /* Enable the UART_TXD function on PTA1 */
    PORTA->PCR[1] = PORT_PCR_MUX(0x2);

    /* Enable the UART_TXD function on PTA2 */
    PORTA->PCR[2] = PORT_PCR_MUX(0x2);
    uart0_init(4000, 9600);// UART0 clock Up to 48 MHz

    Pit_init();

    SIM->SCGC5 |= SIM_SCGC5_PORTE_MASK;
    PORTE->PCR[29] |= PORT_PCR_MUX(1) | PORT_PCR_DSE_MASK;
    PTE->PSOR = GPIO_PSOR_PTSO(1<<29);
    PTE->PDDR = GPIO_PDDR_PDD(1<<29);
    while(1)
    {
            printf("\n\r\n\r\n\r\n\r Press number key for enter MCG Modes	\
\n\r      0. FEI\
\n\r      1. FEE\
\n\r      2. FBI\
\n\r      3. FBE\
\n\r      4. PBE\
\n\r      5. PEE\
\n\r      6. BLPI\
\n\r      7. BLPE\
            ");
        switch(uart0_getchar())
        {
            case '0':
                SIM->CLKDIV1 |= SIM_CLKDIV1_OUTDIV4_MASK | SIM_CLKDIV1_OUTDIV1_MASK;
                Cpu_SetMCG(MCG_MODE_FEI);
                SIM->CLKDIV1 = SIM_CLKDIV1_OUTDIV1(1 - 1) |  // core/system clock = MCGOUTCLK / 1
                SIM_CLKDIV1_OUTDIV4(2 - 1);   // flash/bus clock = core/system / 2
                break;
            case '1':
                SIM->CLKDIV1 |= SIM_CLKDIV1_OUTDIV4_MASK | SIM_CLKDIV1_OUTDIV1_MASK;
                Cpu_SetMCG(MCG_MODE_FEE);
                SIM->CLKDIV1 = SIM_CLKDIV1_OUTDIV1(1 - 1) |  // core/system clock = MCGOUTCLK / 1
                SIM_CLKDIV1_OUTDIV4(2 - 1);   // flash/bus clock = core/system / 2
                break;
            case '2':
                SIM->CLKDIV1 |= SIM_CLKDIV1_OUTDIV4_MASK | SIM_CLKDIV1_OUTDIV1_MASK;
                Cpu_SetMCG(MCG_MODE_FBI);
                SIM->CLKDIV1 = SIM_CLKDIV1_OUTDIV1(1 - 1) |  // core/system clock = MCGOUTCLK / 1
                SIM_CLKDIV1_OUTDIV4(1 - 1);   // flash/bus clock = core/system / 1
                break;
            case '3':
                SIM->CLKDIV1 |= SIM_CLKDIV1_OUTDIV4_MASK | SIM_CLKDIV1_OUTDIV1_MASK;
                Cpu_SetMCG(MCG_MODE_FBE);
                SIM->CLKDIV1 = SIM_CLKDIV1_OUTDIV1(1 - 1) |  // core/system clock = MCGOUTCLK / 1
                SIM_CLKDIV1_OUTDIV4(1 - 1);   // flash/bus clock = core/system / 1
                break;
            case '4':
                SIM->CLKDIV1 |= SIM_CLKDIV1_OUTDIV4_MASK | SIM_CLKDIV1_OUTDIV1_MASK;
                Cpu_SetMCG(MCG_MODE_PBE);
                SIM->CLKDIV1 = SIM_CLKDIV1_OUTDIV1(1 - 1) |  // core/system clock = MCGOUTCLK / 1
                SIM_CLKDIV1_OUTDIV4(1 - 1);   // flash/bus clock = core/system / 1
                break;
            case '5':
                SIM->CLKDIV1 |= SIM_CLKDIV1_OUTDIV4_MASK | SIM_CLKDIV1_OUTDIV1_MASK;
                Cpu_SetMCG(MCG_MODE_PEE);
                SIM->CLKDIV1 = SIM_CLKDIV1_OUTDIV1(1 - 1) |  // core/system clock = MCGOUTCLK / 1
                SIM_CLKDIV1_OUTDIV4(2 - 1);   // flash/bus clock = core/system / 2
                break;
                default:
                printf("\n\r Do not supported!");
                break;
        }
                printf("\n\r\n\r MCG Current mode is %s \n\r", CurMode[Cpu_GetCurrentMCGMode()]);
    }
}

/*
** ===================================================================
**     Method      :  Cpu_SetMCG (component MK60N512MD100)
**
**     Description :
**         This method updates the MCG according the requested clock
**         source setting.
**         This method is internal. It is used by Processor Expert only.
** ===================================================================
*/
static void Cpu_SetMCG(uint8_t TargetMCGMode)
{
  uint8_t NextMCGMode;

  NextMCGMode = Cpu_GetCurrentMCGMode(); /* Identify the currently active MCG mode */
  do {
    NextMCGMode = MCGTransitionMatrix[NextMCGMode][TargetMCGMode]; /* Get the next MCG mode on the path to the target MCG mode */
    switch (NextMCGMode) {             /* Set the next MCG mode on the path to the target MCG mode */
      case MCG_MODE_FEI:
        MCG_FEI();
        break;
      case MCG_MODE_FBI:
        MCG_FBI();
        break;
      case MCG_MODE_BLPI:
        MCG_BLPI();
        break;
      case MCG_MODE_BLPE:
        MCG_BLPE();
        break;
      case MCG_MODE_FBE:
        MCG_FBE();
        break;
      case MCG_MODE_FEE:
        MCG_FEE();
        break;
      case MCG_MODE_PBE:
        MCG_PBE();
        break;
      case MCG_MODE_PEE:
        MCG_PEE();
        break;
      default:
        break;
    }
  } while (TargetMCGMode != NextMCGMode); /* Loop until the target MCG mode is set */
}

/*
** ===================================================================
**     Method      :  Cpu_GetCurrentMCGMode (component MK60N512MD100)
**
**     Description :
**         This method returns the active MCG mode
**         This method is internal. It is used by Processor Expert only.
** ===================================================================
*/
uint8_t Cpu_GetCurrentMCGMode(void)
{
  switch (MCG->C1  & MCG_C1_CLKS_MASK) 
  {
    case  MCG_C1_CLKS(0): // PLL / FLL External
      if(MCG->C6 & MCG_C6_PLLS_MASK)
        return MCG_MODE_PEE;
      else
      {
        if(MCG->C1 & MCG_C1_IREFS_MASK)
            return MCG_MODE_FEI;
        else
            return MCG_MODE_FEE;
      }

    case MCG_C1_CLKS(1): // Internal Clock
      if ((MCG->C2 & MCG_C2_LP_MASK) == MCG_C2_LP_MASK) 
      {
        /* Low power mode is enabled */
        return MCG_MODE_BLPI;
      } 
      else 
      {
        /* Low power mode is disabled */
        return MCG_MODE_FBI;
      }

    case MCG_C1_CLKS(2): // External Clock
      if ((MCG->C2 & MCG_C2_LP_MASK) == MCG_C2_LP_MASK) 
      {
        /* Low power mode is enabled */
        return MCG_MODE_BLPE;
      } 
      else 
      {
          if ((MCG->C6 & MCG_C6_PLLS_MASK) == MCG_C6_PLLS_MASK) 
          {
            /* PLL is selected */
            return MCG_MODE_PBE;
          } 
          else 
          {
            /* FLL is selected */
            return MCG_MODE_FBE;
          }
      }
    default:
      return 0x00U;
  }
}

// switch to FEI 48MHz FLL
void MCG_FEI(void)
{
    MCG->C1 |= MCG_C1_IREFS_MASK; // Internal clock (32.768kHz) for FLL

    MCG->C4 &= ~MCG_C4_DRST_DRS_MASK;
    MCG->C4 |= MCG_C4_DMX32_MASK | MCG_C4_DRST_DRS(1);// 32.768 * 1464 = 47972.352kHz ~ 48MHz
    
    MCG->C1 &= ~MCG_C1_CLKS_MASK; // Output of FLL is selected for MCGOUTCLK

    while((MCG->S & MCG_S_IREFST_MASK) == 0); // wait for Internal clock is selected
    while((MCG->S & MCG_S_CLKST_MASK) != 0); // wait for FLL is selected
}

// switch to FEE 20MHz FLL
void MCG_FEE(void)
{
    MCG->C2 |= MCG_C2_RANGE0(3) |// Very high frequency range selected for the crystal oscillator 
               MCG_C2_EREFS0_MASK |
               MCG_C2_HGO0_MASK ; 
    
    MCG->C1 &= ~MCG_C1_FRDIV_MASK;
    MCG->C1 |= MCG_C1_FRDIV(3); // Divide Factor is 256. 8000 / 256 = 31.25kHz

    MCG->C1 &= ~MCG_C1_IREFS_MASK; // External clock (8MHz) for FLL
    
    MCG->C4 &= ~MCG_C4_DRST_DRS_MASK;
    MCG->C4 &= ~MCG_C4_DMX32_MASK;// 31.25 * 640 = 20000kHz
    
    MCG->C6 &= ~MCG_C6_PLLS_MASK;// select FLL
    
    MCG->C1 &= ~MCG_C1_CLKS_MASK; // Output of FLL is selected for MCGOUTCLK
    
    while((MCG->S & MCG_S_OSCINIT0_MASK) == 0); // wait for osc init
    while((MCG->S & MCG_S_PLLST_MASK) != 0); // wait for FLL
    while((MCG->S & MCG_S_IREFST_MASK) != 0); // wait for External clock is selected
    while((MCG->S & MCG_S_CLKST_MASK) != 0); // wait for FLL is selected
}

// switch to FBI 4MHz Internal clock
void MCG_FBI(void)
{
    MCG->SC &= ~MCG_SC_FCRDIV_MASK; // Divide Factor is 1
    
    MCG->C1 &= ~MCG_C1_CLKS_MASK;
    MCG->C1 |= MCG_C1_IREFS_MASK |// Enable MCGIRCLK for uart0
               MCG_C1_IRCLKEN_MASK;
    
    MCG->C2 &= ~MCG_C2_LP_MASK;  // FLL or PLL is not disabled in bypass modes
    MCG->C2 |= MCG_C2_IRCS_MASK; // Fast internal reference clock selected

    MCG->C6 &= ~MCG_C6_PLLS_MASK;

    MCG->C1 |= MCG_C1_CLKS(1);   // Internal reference clock is selected
    
    while((MCG->S & MCG_S_IRCST_MASK) == 0);
    while((MCG->S & MCG_S_CLKST_MASK) != MCG_S_CLKST(1)); // wait for Internal clock is selected
}

// switch to FBE 8MHz External crystal
void MCG_FBE(void)
{
	  MCG->C6 &= ~MCG_C6_CME0_MASK;

    MCG->C2 |= MCG_C2_RANGE0(3) |// Very high frequency range selected for the crystal oscillator 
               MCG_C2_EREFS0_MASK |
               MCG_C2_HGO0_MASK ; 

    MCG->C4 &= ~MCG_C4_DRST_DRS_MASK;
    MCG->C4 &= ~MCG_C4_DMX32_MASK;
    MCG->C4 |= MCG_C4_DRST_DRS(1);// 32.768 * 1280 = 41943.04kHz
    
    MCG->C6 &= ~MCG_C6_PLLS_MASK;// select FLL
    
    MCG->C1 &= ~MCG_C1_CLKS_MASK;
    MCG->C1 |= MCG_C1_CLKS(2); // Output of FLL is selected for MCGOUTCLK
    
    while((MCG->S & MCG_S_OSCINIT0_MASK) == 0); // wait for osc init
    while((MCG->S & MCG_S_PLLST_MASK) != 0); // wait for FLL
    while((MCG->S & MCG_S_CLKST_MASK) != MCG_S_CLKST(2)); // wait for EXTAL is selected
}

// switch to PBE 8MHz External crystal
void MCG_PBE(void)
{
	  MCG->C6 &= ~MCG_C6_CME0_MASK;

    MCG->C2 &= ~MCG_C2_LP_MASK;
    MCG->C2 |= MCG_C2_RANGE0(3) |// Very high frequency range selected for the crystal oscillator 
               MCG_C2_EREFS0_MASK |
               MCG_C2_HGO0_MASK ; 

    MCG->C5 &= ~MCG_C5_PRDIV0_MASK;
    MCG->C5 |= MCG_C5_PRDIV0(4 - 1); // External clock div 4
    
    MCG->C6 &= ~MCG_C6_VDIV0_MASK;
    MCG->C6 |= MCG_C6_VDIV0(24 - 24) | // Mul 24. 8 / 4 * 24 = 48MHz
               MCG_C6_CME0_MASK |
               MCG_C6_PLLS_MASK;

    MCG->C1 &= ~MCG_C1_CLKS_MASK;
    MCG->C1 |= MCG_C1_CLKS(2); // Output of ExTAL is selected for MCGOUTCLK
    
    while((MCG->S & MCG_S_OSCINIT0_MASK) == 0); // wait for osc init.
    while((MCG->S & MCG_S_PLLST_MASK) != MCG_S_PLLST_MASK); // wait for PLL
    while((MCG->S & MCG_S_CLKST_MASK) != MCG_S_CLKST(2)); // wait for EXTAL is selected
}

// switch to PEE 48MHz PLL
void MCG_PEE(void)
{
	  MCG->C6 &= ~MCG_C6_CME0_MASK;

    MCG->C2 &= ~MCG_C2_LP_MASK;
    MCG->C2 |= MCG_C2_RANGE0(3) |// Very high frequency range selected for the crystal oscillator 
               MCG_C2_EREFS0_MASK |
               MCG_C2_HGO0_MASK ; 

    MCG->C5 &= ~MCG_C5_PRDIV0_MASK;
    MCG->C5 |= MCG_C5_PRDIV0(4 - 1); // External clock div 4
    
    MCG->C6 &= ~MCG_C6_VDIV0_MASK;
    MCG->C6 |= MCG_C6_VDIV0(24 - 24) | // Mul 24. 8 / 4 * 24 = 48MHz
               MCG_C6_CME0_MASK |
               MCG_C6_PLLS_MASK;

    MCG->C1 &= ~MCG_C1_CLKS_MASK; // Output of PLL is selected for MCGOUTCLK
    
    while((MCG->S & MCG_S_OSCINIT0_MASK) == 0); // wait for osc init.
    while((MCG->S & MCG_S_PLLST_MASK) != MCG_S_PLLST_MASK); // wait for PLL
    while((MCG->S & MCG_S_CLKST_MASK) != MCG_S_CLKST(3)); // wait for PLL is selected
}

void MCG_BLPI(void)
{

}

void MCG_BLPE(void)
{

}
