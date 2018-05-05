#include <per_proto.h>
#include <PWM.h>

static uint8_t prescalers_reg[4] = { 0b00, 0b01, 0b10, 0b11 };
static uint8_t prescalers_val[4] = {    1,    4,   16,   64 };

PWM_driver_t    PWMD1 = { .initialized = false };

int PWM_init ( PWM_driver_t *driver, uint16_t pwm_freq )
{
    if ( driver != &PWMD1 )
        return -1;
    
    P1TMRbits.PTMR      = 0;                        // Reset timer counter
    P1TCONbits.PTMOD    = 0x00;                     // Free running mode
    
    uint32_t    max_period      = (1U << 15) - 1;   // Calculate maximum possible period value
    uint32_t    result_period   = 0;
    
    /* Reset driver valibles */
    driver->prescaler_idx   = 0;
    driver->postscaler_idx  = 0;
    
    /* Algorithm to find appropriate period */
    while ( driver->prescaler_idx < 4 )
    {
        // Calculate period with all [pre/post]scalers
        result_period   = (FCY / pwm_freq / prescalers_val[driver->prescaler_idx] / (driver->postscaler_idx + 1)) - 1;

        if ( result_period > max_period )
        {
            /* Check postscaler only in range [1;16] */
            if ( driver->postscaler_idx < 15 )
            {
                driver->postscaler_idx++;
            }
            else
            {
                driver->postscaler_idx = 0;
                driver->prescaler_idx++;
            }
        }
        else
        {
            break;
        }
    }

    if ( driver->prescaler_idx == 4 )
    {
        return -1;
    }
    
    P1TCONbits.PTCKPS       = prescalers_reg[driver->prescaler_idx];    // Prescaler
    P1TCONbits.PTOPS        = driver->postscaler_idx + 1;
            
    P1TPERbits.PTPER        = result_period;
    driver->period_value    = result_period;
    
    P1TCONbits.PTEN         = 1;                                        // Enable PWM
    
    driver->perc2pwm_rate   = result_period / 100.0;
    driver->initialized     = true;
    return 0;
}

int PWM_set_channel_enabled ( PWM_driver_t *driver, PWM_channel_t ch, bool enabled )
{
    if ( driver != &PWMD1 || !driver->initialized )
        return -1;
    
    switch ( ch )
    {
        case PWM_CHANNEL_1L:
        case PWM_CHANNEL_1H:
            PWM1CON1bits.PMOD1 = 1;                     // PWM1 PAIR MODE: Independent Output mode
            break;
        case PWM_CHANNEL_2L:
        case PWM_CHANNEL_2H:
            PWM1CON1bits.PMOD2 = 1;                     // PWM2 PAIR MODE: Independent Output mode
            break;
        case PWM_CHANNEL_3L:
        case PWM_CHANNEL_3H:
            PWM1CON1bits.PMOD3 = 1;                     // PWM3 PAIR MODE: Independent Output mode
            break;
        case PWM_CHANNEL_4L:
        case PWM_CHANNEL_4H:
            PWM1CON1bits.PMOD4 = 1;                     // PWM4 PAIR MODE: Independent Output mode
            break;
        default:
            ;
    } 
    
    switch ( ch )
    {
        case PWM_CHANNEL_1L:
            PWM1CON1bits.PEN1L = enabled ? 1 : 0;       // PWM1L (PE0) pin is enabled for PWM output
            break;
        case PWM_CHANNEL_1H:
            PWM1CON1bits.PEN1H = enabled ? 1 : 0;       // PWM1H (PE1) pin is enabled for PWM output
            break;
        case PWM_CHANNEL_2L:
            PWM1CON1bits.PEN2L = enabled ? 1 : 0;       // PWM2L (PE2) pin is enabled for PWM output
            break;
        case PWM_CHANNEL_2H:
            PWM1CON1bits.PEN2H = enabled ? 1 : 0;       // PWM2H (PE3) pin is enabled for PWM output
            break;
        case PWM_CHANNEL_3L:
            PWM1CON1bits.PEN3L = enabled ? 1 : 0;       // PWM3L (PE4) pin is enabled for PWM output
            break;
        case PWM_CHANNEL_3H:
            PWM1CON1bits.PEN3H = enabled ? 1 : 0;       // PWM3H (PE5) pin is enabled for PWM output
            break;
        case PWM_CHANNEL_4L:
            PWM1CON1bits.PEN4L = enabled ? 1 : 0;       // PWM4L (PE6) pin is enabled for PWM output
            break;
        case PWM_CHANNEL_4H:
            PWM1CON1bits.PEN4H = enabled ? 1 : 0;       // PWM4H (PE7) pin is enabled for PWM output
            break;
        default:
            ;
    }
    
    return 0;
}

void PWM_set_channel_value ( PWM_driver_t *driver, PWM_channel_t ch, uint16_t dc_perc )
{
    if ( driver != &PWMD1 || !driver->initialized )
        return;
    
    if ( dc_perc > 100 )
        dc_perc = 100;
     
    /* TODO - Maybe it is better to use clearly integer calculation? */
    switch ( ch )
    {
        case PWM_CHANNEL_1L:
        case PWM_CHANNEL_1H:
            PDC1 = (int)(dc_perc * driver->perc2pwm_rate) << 1;
            break;
        case PWM_CHANNEL_2L:
        case PWM_CHANNEL_2H:
            PDC2 = (int)(dc_perc * driver->perc2pwm_rate) << 1;
            break;
        case PWM_CHANNEL_3L:
        case PWM_CHANNEL_3H:
            PDC3 = (int)(dc_perc * driver->perc2pwm_rate) << 1;
            break;
        case PWM_CHANNEL_4L:
        case PWM_CHANNEL_4H:
            PDC4 = (int)(dc_perc * driver->perc2pwm_rate) << 1;
            break;
        default:
            ;
    } 
}
