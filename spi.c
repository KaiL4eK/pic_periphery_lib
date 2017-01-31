#include "per_proto.h"

#define CS_TRIS _TRISA0
#define CS_LAT  _LATA0

typedef enum
{
    SPI_SEC_8 = 0b000,
    SPI_SEC_7 = 0b001,
    SPI_SEC_6 = 0b010,
    SPI_SEC_5 = 0b011,
    SPI_SEC_4 = 0b100,
    SPI_SEC_3 = 0b101,
    SPI_SEC_2 = 0b110,
    SPI_SEC_1 = 0b111
}SPI_secondPrescale_t;

void spi_init( uint8_t idle_state )
{
    SPI2STATbits.SPIEN = 0;     // Disable SPI2 module
    
    CS_TRIS = 0;                // Chip select bit as output
    CS_LAT = 1;
    
    IFS2bits.SPI2IF = 0;
    IEC2bits.SPI2IE = 0;
    
    SPI2CON1bits.DISSCK = 0;    // Internal serial clock is enabled
    SPI2CON1bits.DISSDO = 0;    // SDO2 pin is controlled by the module
    SPI2CON1bits.MODE16 = 0;    // Communication is byte-wide (8 bits)
    SPI2CON1bits.MSTEN = 1;     // Master mode enabled
    SPI2CON1bits.SMP = 0;       // 1 = Input data sampled at end of data output time; 
                                // 0 = Input data sampled at middle of data output time
    SPI2CON1bits.CKE = 1;       // 1 = Serial output data changes on transition from active clock state to Idle clock state
                                // 0 = Serial output data changes on transition from Idle clock state to active clock state
    SPI2CON1bits.CKP = idle_state & 0b1;    // 1 = Idle state for clock is a high level; active state is a low level
                                            // 0 = Idle state for clock is a low level; active state is a high level
    SPI2CON1bits.PPRE = SPI_SPEED_LOW;    // 11 = Primary prescale 1:1
                                        // 10 = Primary prescale 4:1
                                        // 01 = Primary prescale 16:1
                                        // 00 = Primary prescale 64:1
    SPI2CON1bits.SPRE = SPI_SEC_8;  // 111 = Secondary prescale 1:1
                                    // 110 = Secondary prescale 2:1
                                    // ...
                                    // 000 = Secondary prescale 8:1
    SPI2STATbits.SPIEN = 1;     // Enable SPI2 module
}

void spi_set_speed( SPI_speed_t speed )
{
    SPI2STATbits.SPIEN = 0;     // Disable SPI2 module
    SPI2CON1bits.PPRE = speed;
    SPI2STATbits.SPIEN = 1;     // Enable SPI2 module
}

void spi_cs_set( uint8_t bit )
{
    CS_LAT = bit;
}

uint8_t spi_write( uint8_t data )
{
    SPI2BUF = data;                    // write to buffer for TX
    while ( SPI2STATbits.SPITBF );
    while ( !SPI2STATbits.SPIRBF );    // wait for transfer to complete
    uint8_t rcv_data = SPI2BUF;
    return rcv_data;                    // read the received value
}

uint8_t spi_read( void )
{
    return( spi_write(0xff) );
}
