#pragma once

/*** Configuration ***/

#define UART_DATA_BUFFER_SIZE       256

/*** Configuration end ***/

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
}UART_module;

extern volatile UART_module uart1_ctx;
extern volatile UART_module uart2_ctx;

typedef uint16_t    UART_baud_rate_t;

// TODO <<<! Count other UART rates
#ifdef DSPIC_ENABLE_PLL
    #define UART_BAUD_RATE_460800_HS    21
    #define UART_BAUD_RATE_921600_HS    10
#else
    #define UART_BAUD_RATE_9600_LS      103
    #define UART_BAUD_RATE_19200_LS     51
    #define UART_BAUD_RATE_38400_LS     25
    #define UART_BAUD_RATE_57600_LS     17
    #define UART_BAUD_RATE_115200_LS    8

    #define UART_BAUD_RATE_57600_HS     68
    #define UART_BAUD_RATE_115200_HS    34
    #define UART_BAUD_RATE_230400_HS    16
    #define UART_BAUD_RATE_460800_HS    8
#endif

/*** Prototypes ***/

/*
 * @brief       Initialization function
 * @param       uart_ctx    Pointer 2 driver context (uart*_ctx)
 * @param       baud_rate   Desired baudrate of interface
 * @param       high_speed  Flag that enables high speed mode
 * @param       priority    Level of interrupt priority
 * @return      0  - everything is Ok
 *              <0 - error happened
 */
int     UART_init( UART_module *uart_ctx, UART_baud_rate_t baud_rate, bool high_speed, Interrupt_priority_lvl_t priority );


void    UART_write_set_big_endian_mode ( UART_module *module, bool big_endian );


void    UART_write_byte( UART_module *module, uint8_t elem );


void    UART_write_words( UART_module *module, uint16_t *arr, uint8_t count );


void    UART_write_string( UART_module *module, const char *fstring, ... );


uint8_t UART_bytes_available( UART_module *module );


void    UART_clean_input( UART_module *module );


uint8_t UART_get_byte( UART_module *module );


void    UART_get_bytes( UART_module *module, uint8_t *out_buffer, uint8_t n_bytes );
