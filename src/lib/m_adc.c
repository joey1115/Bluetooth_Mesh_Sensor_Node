#include "m_adc.h"

// Handler for saadc
static void saadc_evt_handler(nrfx_saadc_evt_t const *p_event);

static nrf_saadc_value_t m_buffer_pool[NUMBER_OF_BUFFERS][SAMPLES_IN_BUFFER];

void saadc_init(void)
{
    ret_code_t err_code;

    nrfx_saadc_config_t saadc_config = NRFX_SAADC_DEFAULT_CONFIG;
    saadc_config.low_power_mode = true;

    nrf_saadc_channel_config_t channel_config = NRFX_SAADC_DEFAULT_CHANNEL_CONFIG_SE(NRF_SAADC_INPUT_AIN0);

    err_code = nrfx_saadc_init(&saadc_config, saadc_evt_handler);
    APP_ERROR_CHECK(err_code);

    err_code = nrfx_saadc_channel_init(0, &channel_config);
    APP_ERROR_CHECK(err_code);

    for (int i = 0; i < NUMBER_OF_BUFFERS; i++)
    {
        err_code = nrfx_saadc_buffer_convert(m_buffer_pool[i], SAMPLES_IN_BUFFER);
        APP_ERROR_CHECK(err_code);

#ifdef DEBUG
        NRF_LOG_INFO("/n Init adc1: %d", m_buffer_pool[i][0]);
#endif
    }
}

void m_saadc_sample(void)
{
    ret_code_t err_code;

    err_code = nrfx_saadc_sample();
    APP_ERROR_CHECK(err_code);
}

static void saadc_evt_handler(nrfx_saadc_evt_t const *p_event)
{
    ret_code_t err_code;
    uint8_t battery_level = 0;

    switch (p_event->type)
    {
    case NRFX_SAADC_EVT_DONE:
    {
        m_stop_saadc_timer();
        m_buck_off();
#ifdef DEBUG
        m_write_led(0);
#endif
        err_code = nrfx_saadc_buffer_convert(p_event->data.done.p_buffer, SAMPLES_IN_BUFFER);
        APP_ERROR_CHECK(err_code);

        battery_level = p_event->data.done.p_buffer[0] / (float)1024 * 3.6 * 10;
#ifdef DEBUG
        for (int i = 0; i < SAMPLES_IN_BUFFER; i++)
        {
            NRF_LOG_INFO("ADC value : %d", p_event->data.done.p_buffer[i]);
        }
        NRF_LOG_INFO("Battery value : %d", battery_level);
        battery_level_update(battery_level);
#endif
        update_adv_bal(battery_level);
    }
    break;

    case NRFX_SAADC_EVT_CALIBRATEDONE:
    {
    }
    break;

    case NRFX_SAADC_EVT_LIMIT:
    {
    }
    break;

    default:
        break;
    }
    return;
}
