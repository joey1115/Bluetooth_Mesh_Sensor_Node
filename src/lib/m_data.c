#include "m_data.h"

static void alpha_beta_filter(void);
static void vectorize(void);
static void check_sensor(void);
static void update_sensor_timer(void);
static void update_seat_timer(void);

static const uint32_t seat_wait[2] = {SEAT_ENTERING_TIC, SEAT_LEAVING_TIC};
static m_data_filter_status_type_t filter_status = FILTER_FIRST;
static uint32_t interval;
static const uint32_t max_interval = MAXIMUM_OF_INTERVAL * SENSOR_EVENT_INTERVAL;
static m_data_data_type_t curr_data;
static m_data_data_type_t prev_data;
static m_data_IR_vector_type_t IR_vector;
static m_data_seat_status_type_t sensor_status;
static int16_t r;
static int16_t prediction;
static uint16_t sum;
static uint8_t i;
static float tolerance = 10;
static float movement = 10;
static float vector_norm;
static m_data_timer_type_t sensor_timer = {(uint32_t)0, SEAT_UNKNOWN};
static m_data_timer_type_t seat_timer = {(uint32_t)0, SEAT_UNKNOWN};

static int32_t IR_sat(int16_t m_data_IR)
{
    return (((m_data_IR + 3072) < 0) ? 0:(((m_data_IR + 3072) >= 8192) ? 8192:(m_data_IR + 3072)));
}

m_data_seat_status_type_t m_check_data(int16_t m_data_IR1, int16_t m_data_IR2, int16_t m_data_IR3, int16_t m_data_IR4, float m_data_temp, uint32_t tic)
{
    // curr_data.x[0] = ((int32_t)m_data_IR1 / 4 + 2048) >> 5;
    // curr_data.x[1] = ((int32_t)m_data_IR2 / 4 + 2048) >> 5;
    // curr_data.x[2] = ((int32_t)m_data_IR3 / 4 + 2048) >> 5;
    // curr_data.x[3] = ((int32_t)m_data_IR4 / 4 + 2048) >> 5;
    curr_data.x[0] = IR_sat(m_data_IR1) >> 5;
    curr_data.x[1] = IR_sat(m_data_IR2) >> 5;
    curr_data.x[2] = IR_sat(m_data_IR3) >> 5;
    curr_data.x[3] = IR_sat(m_data_IR4) >> 5;
    curr_data.temp = m_data_temp;
    curr_data.tic = tic;

    alpha_beta_filter();
    vectorize();
    check_sensor();
    update_sensor_timer();
    update_seat_timer();

#ifdef DEBUG
    NRF_LOG_INFO("IR1: %d", curr_data.x[0]);
    NRF_LOG_INFO("IR2: %d", curr_data.x[1]);
    NRF_LOG_INFO("IR3: %d", curr_data.x[2]);
    NRF_LOG_INFO("IR4: %d", curr_data.x[3]);
    NRF_LOG_INFO("Norm: %d", (int)vector_norm);
    NRF_LOG_INFO("x: %d", IR_vector.x[0]);
    NRF_LOG_INFO("y: %d", IR_vector.x[1]);
    if (sensor_status == SEAT_OCCUPIED)
    {
        NRF_LOG_INFO("DATA: OCCUPIED");
    }
    else
    {
        NRF_LOG_INFO("DATA: EMPTY");
    }
    if (sensor_timer.ref == SEAT_OCCUPIED)
    {
        NRF_LOG_INFO("SENSOR: OCCUPIED");
    }
    else
    {
        NRF_LOG_INFO("SENSOR: EMPTY");
    }
    if (seat_timer.ref == SEAT_OCCUPIED)
    {
        NRF_LOG_INFO("SEAT: OCCUPIED");
    }
    else
    {
        NRF_LOG_INFO("SEAT: EMPTY");
    }
#endif

    prev_data = curr_data;
    return seat_timer.ref;
}

static void alpha_beta_filter(void)
{
    switch (filter_status)
    {
    case FILTER_FIRST:
        filter_status = FILTER_SECOND;
        break;
    case FILTER_SECOND:
        interval = app_timer_cnt_diff_compute(curr_data.tic, prev_data.tic);
        if (interval < max_interval && interval > 0)
        {
            for (i = 0; i < NUM_OF_IR; i++)
            {
                curr_data.v[i] = (curr_data.x[i] - prev_data.x[i]) / (float)interval;
            }
            filter_status = FILTER_NORMAL;
        }
        else
        {
            filter_status = FILTER_SECOND;
        }
        break;
    case FILTER_NORMAL:
        interval = app_timer_cnt_diff_compute(curr_data.tic, prev_data.tic);
        movement = interval * prev_data.v[i];
        if (interval < max_interval && interval > 0 && movement < 256 && movement > -256)
        {
            for (i = 0; i < NUM_OF_IR; i++)
            {
                prediction = prev_data.x[i] + movement;
                prediction = (prediction < 0) ? 0 : ((prediction > 255) ? 255 : prediction);
                r = (int16_t)curr_data.x[i] - prediction;
                if (r <= 1.414 * tolerance)
                {
                    curr_data.x[i] = prediction + alpha * r;
                    curr_data.v[i] = prev_data.v[i] + beta / interval * r;
                }
            }
            filter_status = FILTER_NORMAL;
        }
        else
        {
            filter_status = FILTER_SECOND;
        }
        break;
    default:
        filter_status = FILTER_FIRST;
        break;
    }
}

static void vectorize(void)
{
    sum = 0;
    for (i = 0; i < NUM_OF_IR; i++)
    {
        sum = sum + curr_data.x[i];
    }
    IR_vector.x[0] = ((int16_t)curr_data.x[0] - (int16_t)curr_data.x[1]);
    IR_vector.x[1] = ((int16_t)curr_data.x[1] - (int16_t)curr_data.x[3]);
    uint8_t offset = (curr_data.temp > 30) * 40;
    // IR_vector.cof = powf(4, 1024 / ((float)sum));
    IR_vector.cof = 4.0 / sum;
}

static bool isOccupied()
{
    
    for(i = 0; i < NUM_OF_IR; i++)
    {
        
        if (curr_data.x[i] > 100) {
            return true;
        }
        
    }
    return false;
}

static void check_sensor(void)
{
    if (IR_vector.x[0] == 0 && IR_vector.x[1] == 0)
    {
        vector_norm = IR_vector.cof;
    }
    else
    {
        vector_norm = IR_vector.cof * sqrtf((float)IR_vector.x[0] * (float)IR_vector.x[0] + (float)IR_vector.x[1] * (float)IR_vector.x[1]);
    }

    // if (vector_norm < threshold)
    if (isOccupied())
    {
        sensor_status = SEAT_OCCUPIED;
    }
    else
    {
        sensor_status = SEAT_AVAILABLE;
    }
}

static void update_sensor_timer(void)
{
    if (sensor_timer.ref == SEAT_UNKNOWN)
    {
        sensor_timer.ref = sensor_status;
        sensor_timer.tic = 0;
    }
    else if (sensor_timer.ref == sensor_status)
    {
        sensor_timer.tic = 0;
    }
    else
    {
        sensor_timer.tic += app_timer_cnt_diff_compute(curr_data.tic, prev_data.tic);
        if (sensor_timer.tic >= SENSOR_WAIT_TIC)
        {
            sensor_timer.tic = 0;
            sensor_timer.ref = sensor_status;
        }
    }
}

static void update_seat_timer(void)
{
    if (seat_timer.ref == SEAT_UNKNOWN)
    {
        seat_timer.ref = sensor_timer.ref;
        seat_timer.tic = 0;
    }
    else if (seat_timer.ref == sensor_timer.ref)
    {
        seat_timer.tic = 0;
    }
    else
    {
        seat_timer.tic += app_timer_cnt_diff_compute(curr_data.tic, prev_data.tic);
        if (seat_timer.tic >= seat_wait[seat_timer.ref])
        {
            seat_timer.tic = 0;
            seat_timer.ref = sensor_timer.ref;
        }
    }
}

#ifdef DEBUG
uint16_t geta(void)
{
    return sum;
}

int16_t getx(void)
{
    return IR_vector.x[0];
}

int16_t gety(void)
{
    return IR_vector.x[1];
}

int16_t get1(void)
{
    return prev_data.x[0];
}

int16_t get2(void)
{
    return prev_data.x[1];
}

int16_t get3(void)
{
    return prev_data.x[2];
}

int16_t get4(void)
{
    return prev_data.x[3];
}

float getn(void)
{
    return vector_norm;
}

m_data_data_type_t getData(void)
{
    return curr_data;
}

m_data_IR_vector_type_t getV(void)
{
    return IR_vector;
}
#endif
