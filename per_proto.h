#ifndef PERIPHERY_PROTO_H_
#define	PERIPHERY_PROTO_H_

#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdarg.h>
#include <math.h>
#include <string.h>
#include <xc.h>

#define RADIANS_TO_DEGREES          57.295779513f
#define DEGREES_TO_RADIANS          0.017453293f

#define SWAP( x, y ) { uint8_t tmp = x; x = y; y = tmp; }
#define OFF_ALL_ANALOG_INPUTS   { AD1PCFGL = 0x1fff; }

#define FOSC        32000000ULL
#define FCY         (FOSC/2)

/*** ADC.c ***/

int ADC_init ( uint8_t channel );
int16_t ADC_read( void );

/*** UART.c ***/

typedef void *      uart_module_t;
//typedef uint16_t    UART_speed_t;

typedef enum {
// Low baud rate
    UART_BAUD_IDX_9600               = 0,
    UART_BAUD_IDX_19200              = (UART_BAUD_IDX_9600 + 1),
    UART_BAUD_IDX_38400              = (UART_BAUD_IDX_19200 + 1),
    UART_BAUD_IDX_57600_L            = (UART_BAUD_IDX_38400 + 1),
    UART_BAUD_IDX_115200_L           = (UART_BAUD_IDX_57600_L + 1),
    UART_BAUD_IDX_last_low_speed     = UART_BAUD_IDX_115200_L,
// High Baud Rate
    UART_BAUD_IDX_57600              = (UART_BAUD_IDX_last_low_speed + 1),
    UART_BAUD_IDX_115200             = (UART_BAUD_IDX_57600 + 1),
    UART_BAUD_IDX_230400             = (UART_BAUD_IDX_115200 + 1),
    UART_BAUD_IDX_460800             = (UART_BAUD_IDX_230400 + 1)
} UART_speed_idx_t;
    
typedef enum
{
    INT_PRIO_OFF        = 0,
    INT_PRIO_LOWEST     = 1,
    INT_PRIO_LOW        = 2,
    INT_PRIO_MID_LOW    = 3,
    INT_PRIO_MID        = 4,
    INT_PRIO_MID_HIGH   = 5,
    INT_PRIO_HIGH       = 6,
    INT_PRIO_HIGHEST    = 7
} Interrupt_priority_lvl_t;

uart_module_t   UART_init( uint8_t module, UART_speed_idx_t i_baud, Interrupt_priority_lvl_t priority );
void            UART_write_set_big_endian_mode ( uart_module_t module, bool big_endian );
void            UART_write_byte( uart_module_t module, uint8_t elem );
void            UART_write_words( uart_module_t module, uint16_t *arr, uint8_t count );
void            UART_write_string( uart_module_t module, const char *fstring, ... );
uint8_t         UART_bytes_available( uart_module_t module );
void            UART_clean_input( uart_module_t module );
uint8_t         UART_get_byte( uart_module_t module );
void            UART_get_bytes( uart_module_t module, uint8_t *out_buffer, uint8_t n_bytes );

/*** twi.c ***/

typedef void *      i2c_module_t;

i2c_module_t i2c_init( uint8_t module_num, long Fscl );

int i2c_write_bit( i2c_module_t module, uint8_t slave_addr, uint8_t eeprom_addr, uint8_t bit_start, uint8_t data );
uint8_t i2c_read_bit( i2c_module_t module, uint8_t slave_addr, uint8_t eeprom_addr, uint8_t bit_start );

int i2c_write_bits( i2c_module_t module, uint8_t slave_addr, uint8_t eeprom_addr, uint8_t bit_start, uint8_t length, uint8_t data );
uint8_t i2c_read_bits( i2c_module_t module, uint8_t slave_addr, uint8_t eeprom_addr, uint8_t bit_start, uint8_t length );

int i2c_write_byte( i2c_module_t module, uint8_t slave_addr, uint8_t eeprom_addr, uint8_t data );
uint8_t i2c_read_byte( i2c_module_t module, uint8_t slave_addr, uint8_t eeprom_addr );

int i2c_write_bytes( i2c_module_t module, uint8_t slave_addr, uint8_t eeprom_addr, uint8_t size, uint8_t *data_buffer );
int i2c_read_bytes( i2c_module_t module, uint8_t slave_addr, uint8_t eeprom_addr, uint8_t lenght, uint8_t *data );

int i2c_write_word( i2c_module_t module, uint8_t slave_addr, uint8_t eeprom_addr, uint16_t data );

/*** spi.c ***/

typedef enum
{
    SPI_SPEED_LOW       = 0b00,
    SPI_SPEED_MID       = 0b01,
    SPI_SPEED_HIGH      = 0b10,
    SPI_SPEED_HIGHEST   = 0b11
}SPI_speed_t;

void spi_init( uint8_t idle_state );
uint8_t spi_write( uint8_t data );
uint8_t spi_read( void );
void spi_cs_set( uint8_t bit );
void spi_set_speed( SPI_speed_t primary );

/*** timing.c ***/

void setup_PLL_oscillator( void );

#ifndef DELAY_MS_
#define DELAY_MS_
void delay_ms( uint16_t mseconds );
#endif
#ifndef DELAY_US_
#define DELAY_US_
void delay_us( uint16_t useconds );
#endif

#define TIMER_DIV_1   0b00
#define TIMER_DIV_8   0b01
#define TIMER_DIV_64  0b10
#define TIMER_DIV_256 0b11

/** Timer module **/
void timer_start();
void timer_restart();
void timer_stop();
uint32_t timer_get_us ();
uint32_t timer_get_ms ();

void timer_set_timeout( uint16_t mseconds );
bool timer_is_timeout( void );

/** Input capture module **/

#define IC_TIMER_2  1
#define IC_TIMER_3  0

#define IC_CE_MODE_DISABLED         0b000
#define IC_CE_MODE_EDGE             0b001
#define IC_CE_MODE_FALLING_EDGE     0b010
#define IC_CE_MODE_RISING_EDGE      0b011
#define IC_CE_MODE_4TH_RISE_EDGE    0b100
#define IC_CE_MODE_16TH_RISE_EDGE   0b101

#define IC_INT_MODE_1ST_CE    0b00
#define IC_INT_MODE_2ND_CE    0b01
#define IC_INT_MODE_3RD_CE    0b10
#define IC_INT_MODE_4TH_CE    0b11

/*** flash.c ***/

typedef enum
{
    FILE_NUM
            
}FlashData_t;

int flash_flush ( void );
int flash_read ( void );
int flash_set ( FlashData_t data_type, int data );
int flash_get ( FlashData_t data_type );

#define clip_value( val, min, max ) ((val) > max ? max : (val) < min ? min : (val))

#endif	/* PERIPHERY_PROTO_H_ */

