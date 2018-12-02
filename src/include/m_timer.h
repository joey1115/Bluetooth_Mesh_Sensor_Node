#ifndef __M_TIMER_H__
#define __M_TIMER_H__

#include "m_include.h"

#include "nordic_common.h"
#include "nrf.h"
#include "nrf_sdm.h"
#include "app_error.h"
#include "app_timer.h"

#include "m_config.h"

#ifdef __cplusplus
extern "C"
{
#endif

// #define APP_TIMER_MS(TIC)                                              
//     ((uint32_t)ROUNDED_DIV(                                            
//         (uint32_t)(TIC) * (APP_TIMER_CONFIG_RTC_FREQUENCY + 1) * 1000, 
//         (uint64_t)APP_TIMER_CLOCK_FREQ))

    void timers_init(void);
    void application_timers_start(void);
    void m_stop_saadc_timer(void);

#ifdef __cplusplus
}
#endif

#endif
