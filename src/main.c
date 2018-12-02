#include "m_include.h"

#include "nrf_pwr_mgmt.h"

#define DEAD_BEEF 0xDEADBEEF /**< Value used as error code on stack dump, can be used to identify stack location on stack unwind. */

#include "m_config.h"

/**@brief Function for initializing power management.
 */
void power_management_init(void)
{
    ret_code_t err_code;
    err_code = nrf_pwr_mgmt_init();
    APP_ERROR_CHECK(err_code);
}

/**@brief Callback function for asserts in the SoftDevice.
 *
 * @details This function will be called in case of an assert in the SoftDevice.
 *
 * @warning This handler is an example only and does not fit a final product. You need to analyze
 *          how your product is supposed to react in case of Assert.
 * @warning On assert from the SoftDevice, the system can only recover on reset.
 *
 * @param[in] line_num   Line number of the failing ASSERT call.
 * @param[in] file_name  File name of the failing ASSERT call.
 */
void assert_nrf_callback(uint16_t line_num, const uint8_t *p_file_name)
{
    app_error_handler(DEAD_BEEF, line_num, p_file_name);
}

/**@brief Function for application main entry.
 */
int main(void)
{
    ret_code_t err_code;

    // err_code = sd_power_system_off();
    // APP_ERROR_CHECK(err_code);

    // Initialize.
#ifdef DEBUG
    log_init();
#endif

    timers_init();
    power_management_init();

    m_gpio_init();
    saadc_init();
    AK9750_init();

    ble_stack_init();
    gap_params_init();
    gatt_init();
    advertising_init();
    services_init();
    conn_params_init();
    peer_manager_init();
    // Start execution.
#ifdef DEBUG
    NRF_LOG_INFO("Heart Rate Sensor example started.");
#endif
    application_timers_start();
    advertising_start(false);

    // Enter main loop.
    for (;;)
    {
        err_code = nrf_ble_lesc_request_handler();
        APP_ERROR_CHECK(err_code);
#ifdef DEBUG
        if (NRF_LOG_PROCESS() == false)
        {
#endif
            nrf_pwr_mgmt_run();
#ifdef DEBUG
        }
#endif
    }
}
