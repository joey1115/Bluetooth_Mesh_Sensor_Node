#include "m_ak975x.h"

// Event handler for I2C
static void AK9750_twi_evt_handler(nrfx_twi_evt_t const *p_event, void *p_context);
// Read a 8-bit register
static uint8_t AK9750_readRegister(uint8_t reg_location);
// Write a 8-bit register
static void AK9750_writeRegister(uint8_t reg_location, uint8_t val);
// Read a 8-bit register
static uint16_t AK9750_readRegister16(uint8_t reg_location);
// Initialize I2C
static void AK9750_twi_init(void);
// Enable I2C
static void AK9750_twi_enable(void);
// Disable I2C
static void AK9750_twi_disable(void);
// Soft reset IR Sensor
static void AK9750_soft_reset(void);
// Configure interrupts
static void AK9750_set_interrupts(bool ir13h, bool ir13l, bool ir24h, bool ir24l, bool dr);
// Set cut-off frequency
static void AK9750_set_cutoff_freq(uint8_t freq);
// Set operating mode
static void AK9750_set_mode(uint8_t mode);
// Get the ID of IR sensor module
static void AK9750_get_device_id();
// Refresh IR sensor
static void AK9750_refresh();
// Get the reading of channel 1
static int16_t AK9750_get_IR1(void);
// Get the reading of channel 2
static int16_t AK9750_get_IR2(void);
// Get the reading of channel 3
static int16_t AK9750_get_IR3(void);
// Get the reading of channel 4
static int16_t AK9750_get_IR4(void);
// Get the temperature reading
static float AK9750_get_TMP(void);

static ak9750_sensor_t ir_sensor;
static m_data_seat_status_type_t seat_status;
static m_sensor_operating_mode_type_t operating_mode;
static const uint8_t AK9750_ADDR[NUM_OF_OPERATING_MODE] = {0x64, 0x65, 0x66, 0x00};

void AK9750_read(void)
{
    AK9750_readRegister(AK9750_ST1);
    seat_status = m_check_data(AK9750_get_IR1(), AK9750_get_IR2(), AK9750_get_IR3(), AK9750_get_IR4(), AK9750_get_TMP(), app_timer_cnt_get());
    (seat_status == SEAT_OCCUPIED) ? update_adv_seat((uint8_t)1, AK9750_get_TMP()) : update_adv_seat((uint8_t)0, AK9750_get_TMP());
#ifdef DEBUG
    NRF_LOG_INFO("%d", NRF_FICR->DEVICEID[0]);
    NRF_LOG_INFO("Temp: " NRF_LOG_FLOAT_MARKER, NRF_LOG_FLOAT(AK9750_get_TMP()));
#endif
    AK9750_refresh();
}

static ak9750_sensor_t ir_sensor = {
    NRFX_TWI_INSTANCE(TWI_INSTANCE_ID),
    {AK9750_SCL_PIN,
     AK9750_SDA_PIN,
     NRF_TWI_FREQ_400K,
     AK9750_IRQ_PRIORITY,
     false},
    AK9750_twi_evt_handler};

void AK9750_init(void)
{
    operating_mode = SENSOR_DEFAULT_MODE;
    m_sensor_gpio_config(operating_mode);
    nrf_delay_ms(4);
    AK9750_twi_init();
    AK9750_twi_enable();
    AK9750_soft_reset();
    AK9750_set_mode(AK9750_MODE_1);
    AK9750_set_cutoff_freq(AK9750_FREQ_8_8HZ);
    AK9750_refresh();
}

static void AK9750_twi_evt_handler(nrfx_twi_evt_t const *p_event, void *p_context)
{
    switch (p_event->type)
    {
    case NRFX_TWI_EVT_DONE:
        switch (p_event->xfer_desc.type)
        {
        case NRFX_TWI_XFER_RX:
            /* code */
            break;
        case NRFX_TWI_XFER_TX:
            /* code */
            break;
        default:
            break;
        }
        break;
    default:

        break;
    }
}

static uint8_t AK9750_readRegister(uint8_t reg_location)
{
    ret_code_t err_code;
    uint8_t tx_buf[2];
    uint8_t rx_buf[2];
    tx_buf[0] = reg_location;

    err_code = nrfx_twi_tx(&ir_sensor.twi_instance, AK9750_ADDR[operating_mode], tx_buf, 1, false);
    APP_ERROR_CHECK(err_code);
    if (err_code == NRF_SUCCESS)
    {
        err_code = nrfx_twi_rx(&ir_sensor.twi_instance, AK9750_ADDR[operating_mode], rx_buf, 1);
        APP_ERROR_CHECK(err_code);
    }
    return rx_buf[0];
}

static void AK9750_writeRegister(uint8_t reg_location, uint8_t val)
{
    ret_code_t err_code;
    uint8_t tx_buf[2];
    tx_buf[0] = reg_location;
    tx_buf[1] = val;
    err_code = nrfx_twi_tx(&ir_sensor.twi_instance, AK9750_ADDR[operating_mode], tx_buf, sizeof(tx_buf), false);
    APP_ERROR_CHECK(err_code);
}

static uint16_t AK9750_readRegister16(uint8_t reg_location)
{
    ret_code_t err_code;
    uint8_t tx_buf[2];
    uint8_t rx_buf[2];
    tx_buf[0] = reg_location;
    err_code = nrfx_twi_tx(&ir_sensor.twi_instance, AK9750_ADDR[operating_mode], tx_buf, 1, false);
    APP_ERROR_CHECK(err_code);
    if (err_code == NRF_SUCCESS)
    {
        err_code = nrfx_twi_rx(&ir_sensor.twi_instance, AK9750_ADDR[operating_mode], rx_buf, 2);
        APP_ERROR_CHECK(err_code);
    }
    return (uint16_t)rx_buf[0] + ((uint16_t)rx_buf[1] << 8);
}

static void AK9750_twi_init(void)
{
    ret_code_t err_code;
    // NULL in handler argument mean blocking mode
    err_code = nrfx_twi_init(&ir_sensor.twi_instance, &ir_sensor.twi_config, NULL, NULL);
    APP_ERROR_CHECK(err_code);
}

static void AK9750_twi_enable(void)
{
    nrfx_twi_enable(&ir_sensor.twi_instance);
}

static void AK9750_twi_disable(void)
{
    nrfx_twi_disable(&ir_sensor.twi_instance);
}

static void AK9750_soft_reset(void)
{
    AK9750_writeRegister(AK9750_CNTL2, 0xFF);
}

static void AK9750_set_interrupts(bool ir13h, bool ir13l, bool ir24h, bool ir24l, bool dr)
{
    AK9750_writeRegister(AK9750_EINTEN, 0b11000000 | (ir13h << 4 | ir13l << 3 | ir24h << 2 | ir24l << 1 | dr));
}

static void AK9750_set_cutoff_freq(uint8_t freq)
{
    uint8_t ecntl1_val;
    ecntl1_val = AK9750_readRegister(AK9750_ECNTL1);
    ecntl1_val &= 0b11000111;
    AK9750_writeRegister(AK9750_ECNTL1, ecntl1_val |= (freq << 3));
}

static void AK9750_set_mode(uint8_t mode)
{
    uint8_t ecntl1_val;
    ecntl1_val = AK9750_readRegister(AK9750_ECNTL1);
    ecntl1_val &= 0b11111000;
    // ecntl1_val += 5 << 3;
    ecntl1_val |= mode;
    AK9750_writeRegister(AK9750_ECNTL1, ecntl1_val);
}

static void AK9750_get_device_id()
{
    AK9750_readRegister(AK9750_WIA2);
}

static void AK9750_refresh()
{
    AK9750_readRegister(AK9750_ST2);
}

static float AK9750_get_TMP(void)
{
    int16_t value;
    float temperature;
    value = AK9750_readRegister16(AK9750_TMP);
    value >>= 6;
    temperature = 26.75 + (value * 0.125);
    return temperature;
}

static int16_t AK9750_get_IR1(void)
{
    return AK9750_readRegister16(AK9750_IR1);
}

static int16_t AK9750_get_IR2(void)
{
    return AK9750_readRegister16(AK9750_IR2);
}

static int16_t AK9750_get_IR3(void)
{
    return AK9750_readRegister16(AK9750_IR3);
}

static int16_t AK9750_get_IR4(void)
{
    return AK9750_readRegister16(AK9750_IR4);
}
