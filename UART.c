#include "per_proto.h"

#include <UART.h>

/********************************/
/*              UART            */
/********************************/

#define UART1_TX_FLAG               (1 << 12)
#define UART1_RX_FLAG               (1 << 11)

#define UART2_TX_FLAG               (1 << 15)
#define UART2_RX_FLAG               (1 << 14)

#define UART_STA_DATA_ACCESS_BIT    (1 << 0)
#define UART_STA_BUFFER_FULL_BIT    (1 << 9)

#define SET_REG_BIT(reg, mask)      ( (reg) |= (mask) )
#define RESET_REG_BIT(reg, mask)    ( (reg) &= ~(mask) )

volatile UART_module uart1_ctx = {  .initialized = false, .write_big_endian_mode = false, 
                                    .i_write_head_byte = 0, .i_write_tail_byte = 0, .n_write_bytes_available = 0, .write_overflow = false, 
                                    .i_read_head_byte = 0,  .i_read_tail_byte = 0,  .n_read_bytes_available = 0,  .read_overflow = false,
                                    .reg_read = &U1RXREG, .reg_write = &U1TXREG, .reg_status = &U1STA, .reg_interrupt_flag = &IFS0,
                                    .interrupt_flag_tx_mask = UART1_TX_FLAG, .interrupt_flag_rx_mask = UART1_RX_FLAG };

volatile UART_module uart2_ctx = {  .initialized = false, .write_big_endian_mode = false, 
                                    .i_write_head_byte = 0, .i_write_tail_byte = 0, .n_write_bytes_available = 0, .write_overflow = false, 
                                    .i_read_head_byte = 0,  .i_read_tail_byte = 0,  .n_read_bytes_available = 0,  .read_overflow = false,
                                    .reg_read = &U2RXREG, .reg_write = &U2TXREG, .reg_status = &U2STA, .reg_interrupt_flag = &IFS1,
                                    .interrupt_flag_tx_mask = UART2_TX_FLAG, .interrupt_flag_rx_mask = UART2_RX_FLAG };

static char send_buffer[UART_DATA_BUFFER_SIZE];

int UART_init( UART_module *uart_ctx, UART_baud_rate_t baud_rate, bool high_speed, Interrupt_priority_lvl_t priority )
{
    if ( uart_ctx == &uart1_ctx )
    {  
        U1MODEbits.UARTEN   = 0;            // Bit15 TX, RX DISABLED, ENABLE at end of func
        U1MODEbits.UEN      = 0;            // Bits8,9 TX,RX enabled, CTS,RTS not
        
        if ( high_speed )
            U1MODEbits.BRGH = 1;
        else
            U1MODEbits.BRGH = 0;
        U1BRG               = baud_rate;
        
        _U1TXIF             = 0;
        _U1RXIF             = 0;
        
        _U1TXIE             = 1;            // Enable Tx interrupt
        _U1RXIE             = 1;            // Enable Rx interrupt
        
        _U1TXIP             = priority;
        _U1RXIP             = priority;
        
        U1STAbits.UTXISEL0  = 0;
        U1STAbits.UTXISEL1  = 0;
        
        U1MODEbits.UARTEN   = 1;            // And turn the peripheral on
        U1STAbits.UTXEN     = 1; 
    } 
    else if ( uart_ctx == &uart2_ctx )
    {
        U2MODEbits.UARTEN   = 0;
        U2MODEbits.UEN      = 0;
        
        if ( high_speed )
            U2MODEbits.BRGH = 1;
        else
            U2MODEbits.BRGH = 0;
        U2BRG               = baud_rate;
        
        _U2TXIF             = 0;
        _U2RXIF             = 0;
        
        _U2TXIE             = 1;
        _U2RXIE             = 1;
        
        _U2TXIP             = priority;
        _U2RXIP             = priority;
        
        U2STAbits.UTXISEL0  = 0;
        U2STAbits.UTXISEL1  = 0;
        
        U2MODEbits.UARTEN   = 1;
        U2STAbits.UTXEN     = 1;      
    }
    
    uart_ctx->initialized   = true;
    
    return 0;
}

void UART_write_set_big_endian_mode ( UART_module *module, bool big_endian )
{
    if ( module == NULL )
        return;
    
    module->write_big_endian_mode = big_endian;
}

/**
 * Reading API
 */

void rx_interrupt_handler( volatile UART_module *module )
{
    if ( (*(module->reg_status) & UART_STA_DATA_ACCESS_BIT) && !module->read_overflow ) 
    {
        module->read_buffer[module->i_read_head_byte++] = *module->reg_read;
        
        if ( ++module->n_read_bytes_available == UART_DATA_BUFFER_SIZE ) {
            module->read_overflow = true;
        }
    } else
        RESET_REG_BIT( *(module->reg_interrupt_flag), module->interrupt_flag_rx_mask );
}

void __attribute__( (__interrupt__, auto_psv) ) _U1RXInterrupt()
{
    rx_interrupt_handler( &uart1_ctx );
}

void __attribute__( (__interrupt__, auto_psv) ) _U2RXInterrupt()
{
    rx_interrupt_handler( &uart2_ctx );
}

uint8_t UART_get_byte( UART_module *module )   
{
    if ( module == NULL )
        return 0;
    
    if ( module->n_read_bytes_available == 0 )
        return 0;
    
    module->read_overflow = false;
    
    module->n_read_bytes_available--;
    return module->read_buffer[module->i_read_tail_byte++];
}

void UART_get_bytes( UART_module *module, uint8_t *out_buffer, uint8_t n_bytes )
{
    if ( module == NULL )
        return;
    
    int16_t i = 0;
    for ( i = 0; i < n_bytes; i++ ) {
        if ( UART_bytes_available( module ) == 0 )
            return;

        out_buffer[i] = UART_get_byte( module );
    }
}

uint8_t UART_bytes_available( UART_module *module )
{
    if ( module == NULL )
        return 0;
    
    return module->n_read_bytes_available;
}

void UART_clean_input( UART_module *module )
{
    module->n_read_bytes_available = 0;
    module->i_read_tail_byte       = module->i_read_head_byte;
}

/**
 * Writing API
 */

void tx_interrupt_handler( volatile UART_module *module )
{
    while ( !(*(module->reg_status) & UART_STA_BUFFER_FULL_BIT) )
    {
        if ( module->n_write_bytes_available )
        {
            *module->reg_write = module->write_buffer[module->i_write_tail_byte++];
            module->n_write_bytes_available--;
            module->write_overflow = false;
        } else {
            RESET_REG_BIT( *(module->reg_interrupt_flag), module->interrupt_flag_tx_mask );
            break;
        }
    }
}

void __attribute__( (__interrupt__, auto_psv) ) _U1TXInterrupt()
{
    tx_interrupt_handler( &uart1_ctx );
}

void __attribute__( (__interrupt__, auto_psv) ) _U2TXInterrupt()
{
    tx_interrupt_handler( &uart2_ctx );
};

void UART_write_byte( UART_module *module, uint8_t elem )
{
    if ( module == NULL )
        return;

    while ( module->write_overflow ) { Nop(); }
    
    module->write_buffer[module->i_write_head_byte++] = elem;
    module->n_write_bytes_available++;

    SET_REG_BIT( *(module->reg_interrupt_flag), module->interrupt_flag_tx_mask );
    
    if ( module->n_write_bytes_available == UART_DATA_BUFFER_SIZE )
        module->write_overflow = true;
}

#define HIGH_16( x ) (((x) >> 8) & 0xff)
#define LOW_16( x )  ((x) & 0xff)

void UART_write_words( UART_module *module, uint16_t *arr, uint8_t count )
{
    if ( module == NULL )
        return;
    
    uint16_t iter;
    for ( iter = 0; iter < count; iter++ ) {
        if ( module->write_big_endian_mode )
        {
            UART_write_byte( module, HIGH_16( arr[iter] ) );
            UART_write_byte( module, LOW_16( arr[iter] ) );
        } else {
            UART_write_byte( module, LOW_16( arr[iter] ) );
            UART_write_byte( module, HIGH_16( arr[iter] ) );
        }
    }
}

void UART_write_string( UART_module *module, const char *fstring, ... )
{
    if ( module == NULL )
        return;
    
    int iter = 0;
    va_list str_args;
    
    va_start( str_args, fstring );
    vsprintf( send_buffer, fstring, str_args );
    va_end( str_args );
    
    while( send_buffer[iter] != '\0' )
        UART_write_byte( module, send_buffer[iter++] );
}
