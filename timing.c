#include "per_proto.h"
#include <libpic30.h>

typedef uint32_t TimerTicks32_t;
typedef uint16_t TimerTicks16_t;

void setup_PLL_oscillator( void )
{
#ifdef DSPIC_ENABLE_PLL
/* 000000000 = 2
 * 000000001 = 3
 * 000000010 = 4
 * •
 * 000110000 = 50 (default)
 * •
 * 111111111 = 513
 */
    PLLFBDbits.PLLDIV = 38; // M=40 Fvco = 160Mhz
/* 00 = Output/2
 * 01 = Output/4 (default)
 * 10 = Reserved
 * 11 = Output/8
 */
    CLKDIVbits.PLLPOST = 0b00; // N2=2 Fpost = 80Mhz
/* 
 * 00000 = Input/2 (default)
 * 00001 = Input/3
 * •
 * 11111 = Input/33
 */
    CLKDIVbits.PLLPRE = 6;  // N1=8 Fpre = 4Mhz

    __builtin_write_OSCCONH(0b011); // Initiate Clock Switch to Primary 
                                    // Oscillator with PLL (NOSC=0b011) 
    __builtin_write_OSCCONL(0x01);  // Start clock switching 
    while (OSCCONbits.COSC != 0b011) ;
    while (OSCCONbits.LOCK != 1) ;
#endif
}

/********************************/
/*              DELAYS          */
/********************************/

void delay_ms( uint16_t mseconds )
{
    __delay_ms( mseconds );
    
//    T6CONbits.TON = 0;  // Disable timer
//    T6CONbits.T32 = 1;  // 32-bit timer
//    T6CONbits.TCKPS = TIMER_DIV_1; // Prescale bits 1:1
//    TMR7HLD = 0;
//    TMR6 = 0;
//    T6CONbits.TON = 1;  // Enable timer
//    while( (TMR6 | ((uint32_t)TMR7HLD) << 16) < (mseconds*TIMER_MS_TICK) );
//    TMR7HLD = 0;
//    TMR6 = 0;
//    T6CONbits.TON = 0;
}

void delay_us( uint16_t useconds )
{
    __delay_us( useconds );
    
//    T6CONbits.TON = 0;  // Disable timer
//    T6CONbits.T32 = 1;  // 32-bit timer
//    T6CONbits.TCKPS = TIMER_DIV_1; // Prescale bits 1:1
//    TMR7HLD = 0;
//    TMR6 = 0;
//    T6CONbits.TON = 1;  // Enable timer
//    while( (TMR6 | ((uint32_t)TMR7HLD) << 16) < (useconds*TIMER_US_TICK) );
//    TMR7HLD = 0;
//    TMR6 = 0;
//    T6CONbits.TON = 0;
}

static const float timer_tick_2_ms = 1000.0/FCY;
static const float timer_tick_2_us = 1000000.0/FCY;
static const float timer_ms_2_tick = FCY/1000.0;

TimerTicks32_t  timer_ticks      = 0;
volatile  bool  timeout_happened = true;

void timer_set_timeout( uint16_t mseconds )
{
    TimerTicks32_t timeout_ticks = mseconds * timer_ms_2_tick;
    timeout_happened = false;
    
    T8CONbits.TON   = 0;
    T8CONbits.T32   = 1;
    T8CONbits.TCKPS = TIMER_DIV_1;
    _T9IP           = INT_PRIO_HIGHEST;
    _T9IE           = 1;
    _T9IF           = 0;
    
    PR9 = timeout_ticks >> 16;
    PR8 = timeout_ticks;
    T8CONbits.TON   = 1;
}

bool timer_is_timeout( void )
{
    return timeout_happened;
}

void __attribute__( (__interrupt__, no_auto_psv) ) _T9Interrupt()
{
    timeout_happened = true;
    _T9IE            = 0;
    _T9IF            = 0;
}

// Timer (divider = 1, period = MAX, FCY = 16MHz) counts up to 268 sec
// Timer (divider = 1, period = MAX, FCY = 40MHz) counts up to 53 sec
void timer_start()
{
    T8CONbits.TON   = 0;
    T8CONbits.T32   = 1;
    T8CONbits.TCKPS = TIMER_DIV_1;
    TMR8 = TMR9HLD  = 0;
    
    T8CONbits.TON   = 1;
}

void timer_stop()
{
    timer_ticks = TMR8 | ((uint32_t)TMR9HLD) << 16;
    T8CONbits.TON = 0;
}

void timer_restart()
{
    timer_stop();
    timer_start();
}

uint32_t timer_get_ms ()
{
    return( timer_ticks * timer_tick_2_ms );
}

uint32_t timer_get_us ()
{
    return( timer_ticks * timer_tick_2_us );
}
