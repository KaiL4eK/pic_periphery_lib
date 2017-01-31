#include "per_proto.h"

int ADC_init ( uint8_t channel )
{
    if ( channel > 31 )
        return( -1 );
    
    AD1CON1bits.ADON = 0;
    if ( channel > 15 )
        AD1PCFGH &= ~(1 << (channel-16)); // Analog mode pin setup
    else
        AD1PCFGL &= ~(1 << channel); // Analog mode pin setup
    
    AD1CON1bits.ASAM = 1;        // Auto sample
    AD1CON1bits.SSRC = 0b111;    // Auto convertion
    AD1CON1bits.AD12B = 1;       // 0 = 10 bit ADC; 1 = 12 bit ADC
    AD1CON2 = 0x0000;	
    AD1CON3bits.SAMC = 0b11111;	     // Sample time 
    AD1CON3bits.ADCS = 0b11111111;      // Conversion clock select 	         	
	AD1CHS0 = channel;
    AD1CON1bits.ADON = 1;
    return( 0 );
}

int16_t ADC_res = 0;

int16_t ADC_read( void )
{	
    if ( AD1CON1bits.DONE ) {
        AD1CON1bits.DONE = 0;            // reset DONE bit
        ADC_res = ADC1BUF0;
    }
	return( ADC_res );       			// read ADC1 data      
}

