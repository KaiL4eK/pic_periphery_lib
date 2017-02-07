#include "per_proto.h"

/* I2C1 pins: SDA - RG3; SCL - RG2; */

// HW dependent functions

void i2c_idle( void )
{
    // Wait until I2C Bus is Inactive
    while(I2C1CONbits.SEN || I2C1CONbits.RSEN || I2C1CONbits.PEN || I2C1CONbits.RCEN ||
          I2C1CONbits.ACKEN || I2C1STATbits.TRSTAT)
        Nop();
}

void i2c_start( void )
{
    i2c_idle();                     //Ensure Module is Idle
    I2C1CONbits.SEN = 1;	//Initiate Start condition
    while (I2C1CONbits.SEN);   
}

void i2c_stop( void )
{
    i2c_idle();                     //Ensure Module is Idle
    //initiate stop bit
    I2C1CONbits.PEN = 1;    //Initiate stop bit
    while(I2C1CONbits.PEN);         // Wait for stop condition to finish
}

i2c_module_t i2c_init( uint8_t module_num, long Fscl )
{
   I2C1CONbits.I2CEN = 0;           // Disable I2C Mode
   I2C1BRG = ((100000000.0/Fscl - 13) * FCY / 100000000.0) - 2;
   I2C1CONbits.A10M = 0;            // Disable 10 bit address
   
   I2C1CONbits.DISSLW = 1;          // Disable slew rate control
   IFS1bits.MI2C1IF = 0;            // Clear Interrupt
   I2C1CONbits.I2CEN = 1;           // Enable I2C Mode
   i2c_idle();
   i2c_start();
   i2c_idle();
   i2c_stop();
   i2c_idle();
   
   return NULL;
}

void i2c_restart( void )
{
    i2c_idle();                     //Ensure Module is Idle
    I2C1CONbits.RSEN = 1;	//Initiate restart condition
    while (I2C1CONbits.RSEN);
}

void i2c_ack( void )
{
    i2c_idle();                     //Ensure Module is Idle
    I2C1CONbits.ACKDT = 0;          // Acknowledge data bit, 0 = ACK
    I2C1CONbits.ACKEN = 1;          // Ack data enabled
    while(I2C1CONbits.ACKEN);       // wait for ack data to send on bus
}

void i2c_nack( void )
{
    i2c_idle();                     //Ensure Module is Idle
    I2C1CONbits.ACKDT = 1;          // Acknowledge data bit, 1 = NAK
    I2C1CONbits.ACKEN = 1;          // Ack data enabled
    while(I2C1CONbits.ACKEN);       // wait for ack data to send on bus
}

int i2c_send_byte(uint8_t data)
{
    i2c_idle();                     //Ensure Module is Idle
    I2C1TRN = data;    
    while(I2C1STATbits.TBF);       // wait till complete data is sent from buffer */
    while (I2C1STATbits.ACKSTAT); 
    i2c_idle();
    return( 0 );
}

int i2c_receive_byte(uint8_t *data)
{
    i2c_idle();                     //Ensure Module is Idle
    I2C1CONbits.RCEN = 1;        // Enable data reception
    while(I2C1CONbits.RCEN/* && !I2C1STATbits.BCL*/);
    i2c_idle();
    *data = I2C1RCV;
    return( 0 );
}

int i2c_send_bytes(uint8_t *str, uint8_t lenght)
{
    while(lenght--)                    //Transmit Data Until Pagesize
    {
        i2c_idle();                     //Ensure Module is Idle
        if ( i2c_send_byte(*str) != 0 )       //Write 1 byte
        {
            return( -1 );
        }
        i2c_idle();                 //Wait for Idle bus
        str++;
    }
    return( 0 );
}

int i2c_receive_bytes(uint8_t *str, uint8_t length)
{
    while(length--)
    {
        i2c_idle();                     //Ensure Module is Idle
        if ( i2c_receive_byte( str++ ) != 0 )		//get a single byte
            return( -1 );

        i2c_idle();
        if(length)
            i2c_ack();				//Acknowledge until all read
        else
            i2c_nack();
        i2c_idle();
    }
    return( 0 );
}

/** External API **/

int i2c_write_bytes_eeprom( i2c_module_t module, uint8_t slave_addr, uint8_t eeprom_addr, uint8_t size, uint8_t *data_buffer )
{
    if ( !size )
        return( 0 );
    
    i2c_start();
    i2c_send_byte( slave_addr << 1 | 0x0 );
    i2c_send_byte( eeprom_addr );
    
    i2c_send_bytes( data_buffer, size );
   
    i2c_stop();
    return( 0 );
}

int i2c_write_byte_eeprom( i2c_module_t module, uint8_t slave_addr, uint8_t eeprom_addr, uint8_t data )
{
    return( i2c_write_bytes_eeprom( module, slave_addr, eeprom_addr, 1, &data ) );
}

int i2c_write_word_eeprom( i2c_module_t module, uint8_t slave_addr, uint8_t eeprom_addr, uint16_t data )
{
    i2c_start();                    //Generate Start COndition
    // Function send already has idle inside
    if ( i2c_send_byte( slave_addr << 1 ) != 0 )        //Write Control byte
        return( -1 );

    if ( i2c_send_byte( eeprom_addr ) != 0 )            //Write Low Address
        return( -1 );

    if ( i2c_send_byte( data >> 8 ) != 0 )                 //Write Data
        return( -1 );

    if ( i2c_send_byte( data ) != 0 )                 //Write Data
        return( -1 );

    i2c_stop();                     //Initiate Stop Condition
    return( 0 );
}

int i2c_read_bytes_eeprom( i2c_module_t module, uint8_t slave_addr, uint8_t eeprom_addr, uint8_t lenght, uint8_t *data )
{
    if ( !lenght )
        return( 0 );
    
    i2c_start();                    //Generate Start Condition
    if ( i2c_send_byte( slave_addr << 1 ) != 0 )        //Write Control Byte
    {
        return( -1 );
    }
    if ( i2c_send_byte( eeprom_addr ) != 0 )            //Write start address
    {
        return( -1 );
    }
    i2c_restart();                  //Generate restart condition
    if ( i2c_send_byte( (slave_addr << 1) | 0x1 ) != 0 ) //Write control byte for read
    {
        return( -1 );
    }
    if ( i2c_receive_bytes( data, lenght ) != 0 )             //read Length number of bytes
    {
        return( -1 );
    }
    i2c_stop();                     //Generate Stop
    return( 0 );
}

uint8_t i2c_read_byte_eeprom( i2c_module_t module, uint8_t slave_addr, uint8_t eeprom_addr )
{
    uint8_t data;
    i2c_read_bytes_eeprom( module, slave_addr, eeprom_addr, 1, &data );
    return( data );
}

int i2c_write_bits_eeprom( i2c_module_t module, uint8_t slave_addr, uint8_t eeprom_addr, uint8_t bit_start, uint8_t length, uint8_t data )
{
    //      010 value to write
    // 76543210 bit numbers
    //    xxx   args: bitStart=4, length=3
    // 00011100 mask byte
    // 10101111 original value (sample)
    // 10100011 original & ~mask
    // 10101011 masked | value
    uint8_t data_reg = i2c_read_byte_eeprom( module, slave_addr, eeprom_addr ),
            data_write = data;
    
    uint8_t mask = ((1 << length) - 1) << (bit_start - length + 1);
    data_write <<= (bit_start - length + 1); // shift data into correct position
    data_write &= mask; // zero all non-important bits in data
    data_reg &= ~(mask); // zero all important bits in existing byte
    data_reg |= data_write; // combine data with existing byte
    i2c_write_byte_eeprom( module, slave_addr, eeprom_addr, data_reg );
    return( 0 );
}

int i2c_write_bit_eeprom( i2c_module_t module, uint8_t slave_addr, uint8_t eeprom_addr, uint8_t bit_start, uint8_t data )
{
    return( i2c_write_bits_eeprom( module, slave_addr, eeprom_addr, bit_start, 1, data ) );
}

uint8_t i2c_read_bits_eeprom( i2c_module_t module, uint8_t slave_addr, uint8_t eeprom_addr, uint8_t bit_start, uint8_t length )
{
    if ( !length )
        return( 0 );
    
    uint8_t mask = (1 << length) - 1;
    return( (i2c_read_byte_eeprom( module, slave_addr, eeprom_addr ) >> (bit_start - length + 1)) & mask );
}

uint8_t i2c_read_bit_eeprom( i2c_module_t module, uint8_t slave_addr, uint8_t eeprom_addr, uint8_t bit_start )
{
    return (i2c_read_byte_eeprom( module, slave_addr, eeprom_addr ) >> bit_start) & 0x1;
}


