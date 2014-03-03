/* 
 * File:   Main.c
 * Author: Brian
 *
 * Created on May 20, 2013, 12:27 PM
 *
 * Project: PIC24HJ128GP202_StartUp
 * Purpose: Establish a working build for the PIC24HJ128GP202 for further dev
 *          -includes
 *              -maxing out chip speed
 *              -establish GPIO testing (full output, all high)(full digital output, blink)
 *              -establish Functioning UART interface
 */

#include <stdio.h>
#include <stdlib.h>

#include <Generic.h>
#include <p24hxxxx.h>
#include <uart.h>
#include <pps.h>
#include <outcompare.h>
#include <timer.h>

void StartUp (void);
void Chip_Go_Fast(void);

//
#define INCREMENT_PERIOD 30000
#define TIMER_COUNT     10000
//

_FBS( BWRP_WRPROTECT_OFF )
_FSS( SWRP_WRPROTECT_OFF )
_FGS(GWRP_OFF & GCP_OFF )
_FOSCSEL( FNOSC_FRCPLL  & IESO_OFF )
_FOSC( POSCMD_HS & OSCIOFNC_OFF & IOL1WAY_OFF & FCKSM_CSDCMD )
_FWDT( WDTPOST_PS8192 & WDTPRE_PR32 & WINDIS_OFF & FWDTEN_OFF)
_FPOR( FPWRT_PWR128 & ALTI2C_ON )
_FICD
( ICS_PGD1 & JTAGEN_OFF )

int main(int argc, char** argv) {

    Chip_Go_Fast();     //max out chipspeed
    StartUp();      //run a setup of chosen modules and debug states (see "StartUp.c")

    TRISA = 0;
    TRISB = 0;
    AD1PCFGL = 0xFFFF;
    LATA = 0x0000;
    LATB = 0x0000;
    LATBbits.LATB6 = 1;
    

    PPSUnLock;
    PPSOutput(OUT_FN_PPS_OC1, OUT_PIN_PPS_RP7);
    PPSLock;
\
    OpenOC1(OC_IDLE_CON &
            OC_TIMER2_SRC &
            OC_PWM_FAULT_PIN_DISABLE &
            OC_CONTINUE_PULSE
            ,
            0    //this value doesnt matter in PWM, but cant be larger than value 2
            ,
            1000    //the Timer_Period - This_Number = Duty_Cycle
            );
    
    OpenTimer2(T2_ON &
            T2_IDLE_CON &
            T2_GATE_OFF &
            T2_PS_1_8 &
            T2_SOURCE_INT
            ,
            TIMER_COUNT
            );

    OpenTimer3(T3_ON &
            T3_IDLE_CON &
            T3_GATE_OFF &
            T3_PS_1_256 &
            T3_SOURCE_INT
            ,
            INCREMENT_PERIOD
            );

    ConfigIntTimer2(
            T2_INT_PRIOR_5 &
            T2_INT_ON
            );

//    ConfigIntTimer3(
//            T3_INT_PRIOR_0 &
//            T3_INT_ON
//            );

 
    while(1);
    return (EXIT_SUCCESS);
}

inline void Chip_Go_Fast()      /*Maxs out the chip speed. Blocking*/
{
    // Configure PLL prescaler, PLL postscaler, PLL divisor
        PLLFBD = 41; // M = 43
        CLKDIVbits.PLLPOST = 0; // N2 = 2
        CLKDIVbits.PLLPRE = 0; // N1 = 2
    // Initiate Clock Switch to Internal FRC with PLL (NOSC = 0b001)
        __builtin_write_OSCCONH(0x01);
        __builtin_write_OSCCONL(0x01);
    // Wait for Clock switch to occur
        while (OSCCONbits.COSC != 0b001);
    // Wait for PLL to lock
        while(OSCCONbits.LOCK != 1) {};
    return;
}

//////////
//ISR/////
//////////

void __attribute__ ((auto_psv))     _ISR    _T3Interrupt(void)
{
    static WORD 
    _T3IF = 0;

    if(OC1R >= TIMER_COUNT)
    
    return;
}

//for some reason this breaks PWM
//void __attribute__ ((auto_psv))     _ISR    _T2Interrupt(void)
//{
//    _T2IF = 0;
//
//    return;
//}


//void __attribute__ ((auto_psv))     _ISR    _T1Interrupt(void)
//{
//    _T1IF = 0;          //clear interrupt flag
//
//    LATA ^= 0xFFFF;     //XOR latch with HIGH
//    LATB ^= 0xFFFF;     //XOR latch with HIGH
//
//    return;
//}


