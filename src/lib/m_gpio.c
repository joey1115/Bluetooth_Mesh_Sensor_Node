#include "m_gpio.h"

void m_gpio_init(void)
{
    ret_code_t err_code;

    err_code = nrfx_gpiote_init();
    APP_ERROR_CHECK(err_code);

    nrfx_gpiote_out_config_t config = NRFX_GPIOTE_CONFIG_OUT_SIMPLE(false);

    err_code = nrfx_gpiote_out_init(CAD0_PIN_NUM, &config);
    APP_ERROR_CHECK(err_code);

    err_code = nrfx_gpiote_out_init(CAD1_PIN_NUM, &config);
    APP_ERROR_CHECK(err_code);
#ifdef DEBUG
    err_code = nrfx_gpiote_out_init(LED_PIN_NUM, &config);
    APP_ERROR_CHECK(err_code);
#endif
    err_code = nrfx_gpiote_out_init(BUCK_PIN_NUM, &config);
    APP_ERROR_CHECK(err_code);

    err_code = nrfx_gpiote_out_init(PDN_PIN_NUM, &config);
    APP_ERROR_CHECK(err_code);

    err_code = nrfx_gpiote_out_init(INT_PIN_NUM, &config);
    APP_ERROR_CHECK(err_code);

    nrfx_gpiote_out_clear(LED_PIN_NUM);
    nrfx_gpiote_out_clear(BUCK_PIN_NUM);
    
#ifdef DEBUG
    m_write_led(1);
#endif
    m_buck_on();
}
#ifdef DEBUG
void m_write_led(uint8_t status)
{
    (status == 0) ? nrfx_gpiote_out_clear(LED_PIN_NUM) : nrfx_gpiote_out_set(LED_PIN_NUM);
}
#endif
void m_buck_on(void)
{
    nrfx_gpiote_out_set(BUCK_PIN_NUM);
}

void m_buck_off(void)
{
    nrfx_gpiote_out_clear(BUCK_PIN_NUM);
}

void m_sensor_gpio_config(m_sensor_operating_mode_type_t operating_mode)
{
    switch (operating_mode)
    {
    case NORMAL_MODE_1:
        nrfx_gpiote_out_clear(CAD0_PIN_NUM);
        nrfx_gpiote_out_clear(CAD1_PIN_NUM);
        nrfx_gpiote_out_set(PDN_PIN_NUM);
        nrfx_gpiote_out_set(INT_PIN_NUM);
        break;

    case NORMAL_MODE_2:
        nrfx_gpiote_out_set(CAD0_PIN_NUM);
        nrfx_gpiote_out_clear(CAD1_PIN_NUM);
        nrfx_gpiote_out_set(PDN_PIN_NUM);
        nrfx_gpiote_out_set(INT_PIN_NUM);
        break;

    case NORMAL_MODE_3:
        nrfx_gpiote_out_clear(CAD0_PIN_NUM);
        nrfx_gpiote_out_set(CAD1_PIN_NUM);
        nrfx_gpiote_out_set(PDN_PIN_NUM);
        nrfx_gpiote_out_set(INT_PIN_NUM);
        break;

    case SWITCH_MODE:
        nrfx_gpiote_out_set(CAD0_PIN_NUM);
        nrfx_gpiote_out_set(CAD1_PIN_NUM);
        nrfx_gpiote_out_set(PDN_PIN_NUM);
        nrfx_gpiote_out_set(INT_PIN_NUM);
        break;

    default:
        break;
    }
}
