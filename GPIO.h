#ifndef GPIO_H_
#define	GPIO_H_

#include <xc.h> 

typedef struct {
    volatile unsigned int *tris;
    volatile unsigned int *lat;
    volatile unsigned int *port;
    
}GPIO_port_t;


extern GPIO_port_t GPIOA;
extern GPIO_port_t GPIOB;
extern GPIO_port_t GPIOC;
extern GPIO_port_t GPIOD;
extern GPIO_port_t GPIOE;
extern GPIO_port_t GPIOF;

typedef enum {
    GPIO_OUTPUT,
    GPIO_INPUT
} GPIO_mode_t;

/*
 * @brief   Set GPIO mode (input/output)
 * @param   gpio    Pointer 2 GPIO driver
 * @param   pin     GPIO pin number (numeric) [0, 15]
 * @param   mode    GPIO pin mode
 */
void GPIO_set_pin_mode ( GPIO_port_t *gpio, uint16_t pin, GPIO_mode_t mode );

/*
 * @brief   Set GPIO output state
 * @param   gpio    Pointer 2 GPIO driver
 * @param   pin     GPIO pin number (numeric) [0, 15]
 * @param   enabled Output value of pin - high(true) / low(false)
 */
void GPIO_set_pin ( GPIO_port_t *gpio, uint16_t pin, bool enabled );

/*
 * @brief   Read GPIO input state
 * @param   gpio    Pointer 2 GPIO driver
 * @param   pin     GPIO pin number (numeric) [0, 15]
 * @return  true    - high value input
 *          false   - low value input or error
 */
bool GPIO_read_pin ( GPIO_port_t *gpio, uint16_t pin );

#endif	/* GPIO_H_ */

