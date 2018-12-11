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

// Initialize GPIO
void m_gpio_init(void);
#ifdef DEBUG
void m_write_led(uint8_t status);
#endif
// Turn on the switch on the buck for voltage divider
void m_buck_on(void);
// Turn off the swtich on the buck
void m_buck_off(void);
// Configure the GPIO pins that controll the IR sensor
void m_sensor_gpio_config(m_sensor_operating_mode_type_t operating_mode);

#ifdef __cplusplus
}
#endif

#endif