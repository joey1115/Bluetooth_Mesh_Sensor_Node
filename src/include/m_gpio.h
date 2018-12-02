#ifndef __M_GPIO_H__
#define __M_GPIO_H__

#include "m_include.h"

#include "nrfx_gpiote.h"

#define CAD0_PIN_NUM 13
#define CAD1_PIN_NUM 24
#define LED_PIN_NUM 30
#define BUCK_PIN_NUM 7
#define PDN_PIN_NUM 20
#define INT_PIN_NUM 25

#include "m_config.h"

#ifdef __cplusplus
extern "C"
{
#endif

void m_gpio_init(void);
#ifdef DEBUG
void m_write_led(uint8_t status);
#endif
void m_buck_on(void);
void m_buck_off(void);
void m_sensor_gpio_config(m_sensor_operating_mode_type_t operating_mode);

#ifdef __cplusplus
}
#endif

#endif