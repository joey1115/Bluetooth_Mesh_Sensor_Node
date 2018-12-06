#ifndef __M_DATA_H__
#define __M_DATA_H__

#include "m_include.h"

#define NUM_OF_IR 4
#define SENSOR_WAIT_TIC APP_TIMER_TICKS(SENSOR_WAIT_MS)
#define SEAT_ENTERING_TIC APP_TIMER_TICKS(SEAT_ENTERING_MS)
#define SEAT_LEAVING_TIC APP_TIMER_TICKS(SEAT_LEAVING_MS)

typedef enum
{
    FILTER_FIRST,
    FILTER_SECOND,
    FILTER_NORMAL
} m_data_filter_status_type_t;

typedef enum
{
    SEAT_AVAILABLE,
    SEAT_OCCUPIED,
    SEAT_UNKNOWN
} m_data_seat_status_type_t;

typedef struct
{
    uint8_t x[NUM_OF_IR];
    float temp;
    float v[NUM_OF_IR];
    unsigned long tic;
} m_data_data_type_t;

typedef struct
{
    int16_t x[3];
    float cof;
} m_data_IR_vector_type_t;

typedef struct
{
    uint32_t tic;
    m_data_seat_status_type_t ref;
} m_data_timer_type_t;

#include "m_config.h"

#ifdef __cplusplus
extern "C"
{
#endif

    m_data_seat_status_type_t m_check_data(int16_t m_data_IR1, int16_t m_data_IR2, int16_t m_data_IR3, int16_t m_data_IR4, float m_data_temp, uint32_t tic);

#ifdef DEBUG
    uint16_t geta(void);
    int16_t getx(void);
    int16_t gety(void);
    int16_t get1(void);
    int16_t get2(void);
    int16_t get3(void);
    int16_t get4(void);
    float getn(void);
    m_data_data_type_t getData(void);
    m_data_IR_vector_type_t getV(void);
#endif

#ifdef __cplusplus
}
#endif

#endif
