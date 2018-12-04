#ifndef __M_CONFIG_H__
#define __M_CONFIG_H__

#include "m_adc.h"
#include "m_ak975x.h"
#include "m_BLE.h"
#include "m_data.h"
#include "m_gpio.h"
#include "m_timer.h"
#ifdef DEBUG
#include "m_log.h"
#endif

// BLE
#define NUM_OF_ADV_BYTE 4
#define APP_ADV_INTERVAL 300                                /**< The advertising interval (in units of 0.625 ms. This value corresponds to 187.5 ms). */
#define BATTERY_LEVEL_MEAS_INTERVAL APP_TIMER_TICKS(1000)   /**< Battery level measurement interval (ticks). */
#define BATTERY_LEVEL_EVENT_INTERVAL APP_TIMER_TICKS(1000000) /**< Battery level measurement interval (ticks). */

// ADC
#define NUMBER_OF_BUFFERS 2
#define SAMPLES_IN_BUFFER 5

// Filter
#define alpha 0.5
#define beta 0.1
#define MAXIMUM_OF_INTERVAL 5
#define threshold 500
#define SENSOR_WAIT_MS 5000
#define SEAT_ENTERING_MS 10000
#define SEAT_LEAVING_MS 100000

// Sensor
#define SENSOR_DEFAULT_MODE NORMAL_MODE_2
#define SENSOR_EVENT_INTERVAL APP_TIMER_TICKS(1000) /**< Battery level measurement interval (ticks). */
// #define AK9750_DEFAULT_ADDRESS 0x64                 //7-bit unshifted default I2C Address
//Address is changeable via two jumpers on the rear of the PCB.
//Allowed settings are:
//00 (0x64 default)
//10 (0x65)
//01 (0x66)
//11 Not allowed - used for switch mode
// #define CAD0_PIN_VALUE 0
// #define CAD1_PIN_VALUE 0

#ifdef __cplusplus
extern "C"
{
#endif

#ifdef __cplusplus
}
#endif

#endif
