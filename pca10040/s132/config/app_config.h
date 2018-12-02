#ifndef APP_CONFIG_H
#define APP_CONFIG_H

// <e> NRF_LOG_ENABLED - nrf_log - Logger
//==========================================================
#ifdef DEBUG
#define NRF_LOG_ENABLED 1
#else
#define NRF_LOG_ENABLED 0
#endif

#endif