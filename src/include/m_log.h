#ifndef __M_LOG_H__
#define __M_LOG_H__

#include "m_include.h"

#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"

#include "m_config.h"

#ifdef __cplusplus
extern "C" {
#endif

void log_init(void);

#ifdef __cplusplus
}
#endif

#endif