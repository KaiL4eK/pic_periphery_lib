#include <per_proto.h>

#ifdef DSPIC_ENABLE_PLL
    #include <pragmas_pll.h>
#else
    #include <pragmas.h>
#endif

#include <UART.h>

int main ( void ) 
{
    OFF_ALL_ANALOG_INPUTS;
    
    while ( 1 )
    {
        
    }
    
    return 0;
}
