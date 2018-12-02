#ifndef __M_AK975X_H__
#define __M_AK975X_H__

#include "m_include.h"

#include <nrfx.h>
#include <hal/nrf_twi.h>
#include "nrfx_twi.h"
#include "app_util_platform.h"

#define AK9750_SCL_PIN 27
#define AK9750_SDA_PIN 26
#define AK9750_IRQ_PRIORITY APP_IRQ_PRIORITY_HIGH
#define TWI_INSTANCE_ID 0

#define I2C_SPEED_STANDARD 100000
#define I2C_SPEED_FAST 400000

//Register addresses
#define AK9750_WIA2 0x01  //Device ID
#define AK9750_INTST 0x04 //Device ID
#define AK9750_ST1 0x05
#define AK9750_IR1 0x06
#define AK9750_IR2 0x08
#define AK9750_IR3 0x0A
#define AK9750_IR4 0x0C
#define AK9750_TMP 0x0E
#define AK9750_ST2 0x10 //Dummy register
#define AK9750_ETH13H_LOW 0x11
#define AK9750_ETH13H_HIGH 0x12
#define AK9750_ETH13L_LOW 0x13
#define AK9750_ETH13L_HIGH 0x14
#define AK9750_ETH24H_LOW 0x15
#define AK9750_ETH24H_HIGH 0x16
#define AK9750_ETH24L_LOW 0x17
#define AK9750_ETH24L_HIGH 0x18
#define AK9750_EHYS13 0x19
#define AK9750_EHYS24 0x1A
#define AK9750_EINTEN 0x1B
#define AK9750_ECNTL1 0x1C
#define AK9750_CNTL2 0x19
#define AK9750_EKEY 0x50

//Valid sensor modes - Register ECNTL1
#define AK9750_MODE_STANDBY 0b000
#define AK9750_MODE_EEPROM_ACCESS 0b001
#define AK9750_MODE_SINGLE_SHOT 0b010
#define AK9750_MODE_0 0b100
#define AK9750_MODE_1 0b101
#define AK9750_MODE_2 0b110
#define AK9750_MODE_3 0b111

//Valid digital filter cutoff frequencies
#define AK9750_FREQ_0_3HZ 0b000
#define AK9750_FREQ_0_6HZ 0b001
#define AK9750_FREQ_1_1HZ 0b010
#define AK9750_FREQ_2_2HZ 0b011
#define AK9750_FREQ_4_4HZ 0b100
#define AK9750_FREQ_8_8HZ 0b101

//EEPROM addresses
#define AK9750_EETH13H_LOW 0X51
#define AK9750_EETH13H_HIGH 0X52
#define AK9750_EETH13L_LOW 0X53
#define AK9750_EETH13L_HIGH 0X54
#define AK9750_EETH24H_LOW 0X55
#define AK9750_EETH24H_HIGH 0X56
#define AK9750_EETH24L_LOW 0X57
#define AK9750_EETH24L_HIGH 0X58
#define AK9750_EEHYS13 0X59
#define AK9750_EEHYS24 0X5A
#define AK9750_EEINTEN 0X5B

//EEPROM functions
#define EEPROM_MODE 0b11000001
#define EKEY_ON 0b10100101 // 0b10100101=0xA5

typedef enum
{
    NORMAL_MODE_1,
    NORMAL_MODE_2,
    NORMAL_MODE_3,
    SWITCH_MODE,
    NUM_OF_OPERATING_MODE
} m_sensor_operating_mode_type_t;

typedef struct
{
    nrfx_twi_t twi_instance;
    nrfx_twi_config_t twi_config;
    void (*twi_event_handler)(nrfx_twi_evt_t const *, void *);
} ak9750_sensor_t;

#include "m_config.h"

#ifdef __cplusplus
extern "C"
{
#endif

    void AK9750_init(void);
    void AK9750_read(void);

#ifdef __cplusplus
}
#endif

#endif
