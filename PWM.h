#ifndef PWM_H_
#define	PWM_H_


typedef enum {
    PWM_CHANNEL_1L,
    PWM_CHANNEL_1H,
    PWM_CHANNEL_2L,
    PWM_CHANNEL_2H,
    PWM_CHANNEL_3L,
    PWM_CHANNEL_3H,
    PWM_CHANNEL_4L,
    PWM_CHANNEL_4H,
            
} PWM_channel_t;

typedef struct {
    uint8_t     prescaler_idx;
    uint8_t     postscaler_idx;
    uint16_t    period_value;
    float       perc2pwm_rate;
    
    bool        initialized;
} PWM_driver_t;

extern PWM_driver_t    PWMD1;

/*
 * @brief   Initialize PWM driver
 * @param   driver      Driver pointer (PWMD*)
 * @param   pwm_freq    PWM desired frequency
 * @return  <0          Error happened
 *          0           Initialized
 */
int PWM_init ( PWM_driver_t *driver, uint16_t pwm_freq );

/*
 * @brief   Enable/Disable PWM channel
 * @param   driver      Driver pointer (PWMD*)
 * @param   ch          PWM channel of type <PWM_channel_t>
 * @param   enabled     Set channel enabled(true)/disabled(false)
 * @return  <0          Error happened
 *          0           Initialized
 */
int PWM_set_channel_enabled ( PWM_driver_t *driver, PWM_channel_t ch, bool enabled );

/*
 * @brief   Set PWM duty cycle in percentage
 * @param   driver      Driver pointer (PWMD*)
 * @param   ch          PWM channel of type <PWM_channel_t>
 * @param   dc_perc     Duty cycle percantage value [0; 100]
 */
void PWM_set_channel_value ( PWM_driver_t *driver, PWM_channel_t ch, uint16_t dc_perc );

#endif	/* PWM_H_ */

