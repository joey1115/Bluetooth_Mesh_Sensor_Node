#ifndef __M_ADC_H__
#define __M_ADC_H__

#include "m_include.h"

#include "nrfx_saadc.h"

#include "m_config.h"

#ifdef __cplusplus
extern "C" {
#endif

// Initialize SAADC
void saadc_init(void);
// Start ADC sampling once (Nonblocking)
void m_saadc_sample(void);

#ifdef __cplusplus
}
#endif

#endif
