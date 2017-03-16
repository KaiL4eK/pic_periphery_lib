#include <p33FJ256MC710.h>

#include "per_proto.h"

/********************************/
/*              UART            */
/********************************/

#define UART_DATA_BUFFER_SIZE       256

#define UART1_TX_FLAG               (1 << 12)
#define UART1_RX_FLAG               (1 << 11)

#define UART2_TX_FLAG               (1 << 15)
#define UART2_RX_FLAG               (1 << 14)

#define UART_STA_DATA_ACCESS_BIT    (1 << 0)
#define UART_STA_BUFFER_FULL_BIT    (1 << 9)

#define SET_REG_BIT(reg, mask)      ( (reg) |= (mask) )
#define RESET_REG_BIT(reg, mask)    ( (reg) &= ~(mask) )

typedef struct
{
    bool                    initialized;
    
    bool                    write_big_endian_mode;
    uint8_t                 write_buffer[UART_DATA_BUFFER_SIZE];
    uint8_t                 i_write_head_byte;
    uint8_t                 i_write_tail_byte;
    uint8_t                 n_write_bytes_available;
    bool                    write_overflow;
    
    uint8_t                 read_buffer[UART_DATA_BUFFER_SIZE];
    uint8_t                 i_read_head_byte;
    uint8_t                 i_read_tail_byte;
    uint8_t                 n_read_bytes_available;
    bool                    read_overflow;
    
    volatile unsigned int   *reg_read;    
    volatile unsigned int   *reg_write;  
    volatile unsigned int   *reg_status;
    
    volatile unsigned int   *reg_interrupt_flag;
    
    unsigned int            interrupt_flag_tx_mask;
    unsigned int            interrupt_flag_rx_mask;
}UART_module_fd;

volatile UART_module_fd  uart_fd[] = {  {   .initialized = false,
                                            .write_big_endian_mode = false, .i_write_head_byte = 0, .i_write_tail_byte = 0, .n_write_bytes_available = 0, .write_overflow = false, 
                                                .i_read_head_byte = 0,  .i_read_tail_byte = 0,  .n_read_bytes_available = 0,  .read_overflow = false,
                                                .reg_read = &U1RXREG, .reg_write = &U1TXREG, .reg_status = &U1STA, .reg_interrupt_flag = &IFS0,
                                                .interrupt_flag_tx_mask = UART1_TX_FLAG, .interrupt_flag_rx_mask = UART1_RX_FLAG },
                                        {   .initialized = false,
                                            .write_big_endian_mode = false, .i_write_head_byte = 0, .i_write_tail_byte = 0, .n_write_bytes_available = 0, .write_overflow = false, 
                                                .i_read_head_byte = 0,  .i_read_tail_byte = 0,  .n_read_bytes_available = 0,  .read_overflow = false,
                                                .reg_read = &U2RXREG, .reg_write = &U2TXREG, .reg_status = &U2STA, .reg_interrupt_flag = &IFS1,
                                                .interrupt_flag_tx_mask = UART2_TX_FLAG, .interrupt_flag_rx_mask = UART2_RX_FLAG }    };

static inline bool UART_low_speed( UART_speed_t i_baud )
{
    return i_baud <= UART_last_low_speed;
}

#define ASSERT_MODULE_NUMBER( x )   ( (x) == 1 || (x) == 2 )

uart_module_t UART_init( uint8_t module, UART_speed_t i_baud, Interrupt_priority_lvl_t priority )
{
    UART_module_fd *u_module = NULL;
    
    if ( !ASSERT_MODULE_NUMBER( module ) )
        return NULL;
    
    if ( module == 1 )
    {
        u_module = (UART_module_fd *)&uart_fd[0];
        
        U1MODEbits.UARTEN   = 0;            // Bit15 TX, RX DISABLED, ENABLE at end of func
        U1MODEbits.UEN      = 0;            // Bits8,9 TX,RX enabled, CTS,RTS not
        
        if ( UART_low_speed( i_baud ) )
            U1MODEbits.BRGH = 0;
        else
            U1MODEbits.BRGH = 1;
        U1BRG               = uart_speed[i_baud];
        
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
    else
    {
        u_module = (UART_module_fd *)&uart_fd[1];
        
        U2MODEbits.UARTEN   = 0;
        U2MODEbits.UEN      = 0;
        
        if ( UART_low_speed( i_baud ) )
            U2MODEbits.BRGH = 0;
        else
            U2MODEbits.BRGH = 1;
        U2BRG               = uart_speed[i_baud];
        
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
    
    u_module->initialized   = true;
    return u_module;
}

void UART_write_set_big_endian_mode ( uart_module_t module, bool big_endian )
{
    UART_module_fd *u_module = module;
    
    if ( module == NULL )
        return;
    
    u_module->write_big_endian_mode = big_endian;
}

/**
 * Reading API
 */

void rx_interrupt_handler( volatile UART_module_fd  *u_module )
{
    if ( (*(u_module->reg_status) & UART_STA_DATA_ACCESS_BIT) && !u_module->read_overflow ) 
    {
        u_module->read_buffer[u_module->i_read_head_byte++] = *u_module->reg_read;
        
        if ( ++u_module->n_read_bytes_available == UART_DATA_BUFFER_SIZE ) {
            u_module->read_overflow = true;
        }
    } else
        RESET_REG_BIT( *(u_module->reg_interrupt_flag), u_module->interrupt_flag_rx_mask );
}

void __attribute__( (__interrupt__, auto_psv) ) _U1RXInterrupt()
{
    rx_interrupt_handler( &uart_fd[0] );
}

void __attribute__( (__interrupt__, auto_psv) ) _U2RXInterrupt()
{
    rx_interrupt_handler( &uart_fd[1] );
}

uint8_t UART_get_byte( uart_module_t module )   
{
    if ( module == NULL )
        return 0;
    
    UART_module_fd *u_module = module;
    
    if ( u_module->n_read_bytes_available == 0 )
        return 0;
    
    u_module->read_overflow = false;
    
    u_module->n_read_bytes_available--;
    return u_module->read_buffer[u_module->i_read_tail_byte++];
}

void UART_get_bytes( uart_module_t module, uint8_t *out_buffer, uint8_t n_bytes )
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

uint8_t UART_bytes_available( uart_module_t module )
{
    if ( module == NULL )
        return 0;
    
    return ((UART_module_fd *)module)->n_read_bytes_available;
}

void UART_clean_input( uart_module_t module )
{
    UART_module_fd *u_module = module;
    
    u_module->n_read_bytes_available = 0;
    u_module->i_read_tail_byte       = u_module->i_read_head_byte;
}

/**
 * Writing API
 */

void tx_interrupt_handler( volatile UART_module_fd  *u_module )
{
    while ( !(*(u_module->reg_status) & UART_STA_BUFFER_FULL_BIT) )
    {
        if ( u_module->n_write_bytes_available )
        {
            *u_module->reg_write = u_module->write_buffer[u_module->i_write_tail_byte++];
            u_module->n_write_bytes_available--;
            u_module->write_overflow = false;
        } else {
            RESET_REG_BIT( *(u_module->reg_interrupt_flag), u_module->interrupt_flag_tx_mask );
            break;
        }
    }
}

void __attribute__( (__interrupt__, auto_psv) ) _U1TXInterrupt()
{
    tx_interrupt_handler( &uart_fd[0] );
}

void __attribute__( (__interrupt__, auto_psv) ) _U2TXInterrupt()
{
    tx_interrupt_handler( &uart_fd[1] );
};

void UART_write_byte( uart_module_t module, uint8_t elem )
{
    if ( module == NULL )
        return;
    
    UART_module_fd *u_module = module;
    
    while ( u_module->write_overflow ) { Nop(); }
    
    u_module->write_buffer[u_module->i_write_head_byte++] = elem;
    u_module->n_write_bytes_available++;

    SET_REG_BIT( *(u_module->reg_interrupt_flag), u_module->interrupt_flag_tx_mask );
    
    if ( u_module->n_write_bytes_available == UART_DATA_BUFFER_SIZE )
        u_module->write_overflow = true;
}

#define HIGH_16( x ) (((x) >> 8) & 0xff)
#define LOW_16( x )  ((x) & 0xff)

void UART_write_words( uart_module_t module, uint16_t *arr, uint8_t count )
{
    if ( module == NULL )
        return;
    
    UART_module_fd *u_module = module;
    
    uint16_t iter = 0;
    for ( iter = 0; iter < count; iter++ ) {
        if ( u_module->write_big_endian_mode )
        {
            UART_write_byte( module, HIGH_16( arr[iter] ) );
            UART_write_byte( module, LOW_16( arr[iter] ) );
        } else {
            UART_write_byte( module, LOW_16( arr[iter] ) );
            UART_write_byte( module, HIGH_16( arr[iter] ) );
        }
    }
}

static char send_buffer[UART_DATA_BUFFER_SIZE];

void UART_write_string( uart_module_t module, const char *fstring, ... )
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
