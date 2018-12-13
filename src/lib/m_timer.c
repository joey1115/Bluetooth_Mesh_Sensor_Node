#include "m_timer.h"

// Battery level measurement handler
static void battery_level_meas_timeout_handler(void *p_context);
// Battery level event handler
static void battery_level_event_timeout_handler(void *p_context);
// Sensor event handler
static void sensor_event_timeout_handler(void *p_context);

APP_TIMER_DEF(m_battery_timer_id);
APP_TIMER_DEF(m_battery_evt_timer_id);    /**< Sensor contact detected timer. */
APP_TIMER_DEF(m_sensor_evt_timer_id);

void m_stop_saadc_timer(void)
{
    ret_code_t err_code;

    err_code = app_timer_stop(m_battery_timer_id);
    APP_ERROR_CHECK(err_code);
}

/**@brief Function for the Timer initialization.
 *
 * @details Initializes the timer module. This creates and starts application timers.
 */
void timers_init(void)
{
    ret_code_t err_code;

    // Initialize timer module.
    err_code = app_timer_init();
    APP_ERROR_CHECK(err_code);

    // Create timers.
    err_code = app_timer_create(&m_battery_timer_id, APP_TIMER_MODE_REPEATED, battery_level_meas_timeout_handler);
    APP_ERROR_CHECK(err_code);

    err_code = app_timer_create(&m_battery_evt_timer_id, APP_TIMER_MODE_REPEATED, battery_level_event_timeout_handler);
    APP_ERROR_CHECK(err_code);

    err_code = app_timer_create(&m_sensor_evt_timer_id, APP_TIMER_MODE_REPEATED, sensor_event_timeout_handler);
    APP_ERROR_CHECK(err_code);
}

/**@brief Function for starting application timers.
 */
void application_timers_start(void)
{
    ret_code_t err_code;

    err_code = app_timer_start(m_battery_timer_id, BATTERY_LEVEL_MEAS_INTERVAL, NULL);
    APP_ERROR_CHECK(err_code);

    err_code = app_timer_start(m_battery_evt_timer_id, BATTERY_LEVEL_EVENT_INTERVAL, NULL);
    APP_ERROR_CHECK(err_code);

    err_code = app_timer_start(m_sensor_evt_timer_id, SENSOR_EVENT_INTERVAL, NULL);
    APP_ERROR_CHECK(err_code);
}

static void battery_level_meas_timeout_handler(void *p_context)
{
    UNUSED_PARAMETER(p_context);
    m_saadc_sample();
}

static void battery_level_event_timeout_handler(void *p_context)
{
    ret_code_t err_code;

    UNUSED_PARAMETER(p_context);
#ifdef DEBUG
    m_write_led(1);
#endif
    m_buck_on();

    err_code = app_timer_start(m_battery_timer_id, BATTERY_LEVEL_MEAS_INTERVAL, NULL);
    APP_ERROR_CHECK(err_code);
}

static void sensor_event_timeout_handler(void *p_context)
{
    UNUSED_PARAMETER(p_context);
    AK9750_read();
}
