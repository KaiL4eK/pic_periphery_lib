#include <per_proto.h>

#include "GPIO.h"

typedef struct {
    volatile unsigned int *tris;
    volatile unsigned int *lat;
    volatile unsigned int *port;
    
}GPIO_port_t;

GPIO_port_t GPIOA = { .tris = &TRISA, .lat = &LATA, .port = &PORTA };
GPIO_port_t GPIOB = { .tris = &TRISB, .lat = &LATB, .port = &PORTB };
GPIO_port_t GPIOC = { .tris = &TRISC, .lat = &LATC, .port = &PORTC };
GPIO_port_t GPIOD = { .tris = &TRISD, .lat = &LATD, .port = &PORTD };
GPIO_port_t GPIOE = { .tris = &TRISE, .lat = &LATE, .port = &PORTE };
GPIO_port_t GPIOF = { .tris = &TRISF, .lat = &LATF, .port = &PORTF };

void GPIO_set_pin_mode ( GPIO_port_t *gpio, uint16_t pin, GPIO_mode_t mode )
{
    if ( gpio == NULL || pin > 15 )
        return;
    
    if ( mode == GPIO_OUTPUT )
        *(gpio->tris) &= ~(1 << pin);      // Clear bit
    else if ( mode == GPIO_INPUT )
        *(gpio->tris) |= (1 << pin);       // Set bit
}

void GPIO_set_pin_value ( GPIO_port_t *gpio, uint16_t pin, bool enabled )
{
    if ( gpio == NULL || pin > 15 )
        return;
    
    if ( enabled )
        *(gpio->lat) |= (1 << pin);
    else
        *(gpio->lat) &= ~(1 << pin);
}

bool GPIO_read_pin ( GPIO_port_t *gpio, uint16_t pin )
{
    if ( gpio == NULL || pin > 15 )
        return false;
    
    return ((*(gpio->port) >> pin) & 0x1);
}
