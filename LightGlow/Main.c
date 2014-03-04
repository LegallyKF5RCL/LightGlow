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

static WORD Direction = 0;  //this needs to made into the label thing that i forgot about because I havent programmed in so long because ANALOG CLASSESSSSSSSSFACKKKKKKK


//
#define INCREMENT_PERIOD 30000
#define TIMER_COUNT     100
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
    //LATBbits.LATB6 = 1;
    

    PPSUnLock;
    PPSOutput(OUT_FN_PPS_OC1, OUT_PIN_PPS_RP7);
    PPSLock;
\
    OpenOC1(OC_IDLE_CON &
            OC_TIMER2_SRC &
            OC_PWM_FAULT_PIN_ENABLE &
            OC_CONTINUE_PULSE
            ,
            0    //this value doesnt matter in PWM, but cant be larger than value 2
            ,
            95    //the Timer_Period - This_Number = Duty_Cycle
            );
    
    

    OpenTimer1(
            T1_ON &
            T1_IDLE_CON &
            T1_GATE_OFF &
            T1_PS_1_256 &
            T1_SYNC_EXT_OFF &
            T1_SOURCE_INT
            ,
            3000       //bout a quarter second
            );

    ConfigIntTimer1(
            T1_INT_PRIOR_2 &
            T1_INT_ON
            );

    OpenTimer2(
            T2_ON &
            T2_IDLE_CON &
            T2_GATE_OFF &
            T2_PS_1_8 &
            T2_SOURCE_INT
            ,
            TIMER_COUNT
            );

    ConfigIntTimer2(
            T2_INT_PRIOR_5 &
            T2_INT_ON
            );


    

    //OC1R = 5000;

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


//for some reason this breaks PWM
void __attribute__ ((auto_psv))     _ISR    _T2Interrupt(void)
{
    _T2IF = 0;

    return;
}


void __attribute__ ((auto_psv))     _ISR    _T1Interrupt(void)
{
    _T1IF = 0;          //clear interrupt flag

    //LATA ^= 0xFFFF;

    if(Direction == 0)
    {
        OC1R = OC1R - 1;
        if(OC1R <= (TIMER_COUNT >> 2))
        {
            Direction = 1;
        }
    }
    else if(Direction == 1)
    {
        OC1R = OC1R + 1;
        if(OC1R >= (TIMER_COUNT))
        {
            Direction = 0;
        }
    }

    return;
}


