#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "nrf.h"
#include "nrf_drv_saadc.h"
#include "nrf_drv_ppi.h"
#include "nrf_drv_timer.h"
#include "boards.h"
#include "app_error.h"
#include "nrf_delay.h"
#include "app_util_platform.h"
#include "nrf_pwr_mgmt.h"

#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"

#include "common.h"

#define SAMPLES_IN_BUFFER 10
#define SAMPLE_PERIOD 300

static const nrf_drv_timer_t m_timer = NRF_DRV_TIMER_INSTANCE(3);
static nrf_saadc_value_t     m_buffer_pool[2][SAMPLES_IN_BUFFER];
static nrf_ppi_channel_t     m_ppi_channel;
static uint32_t              m_adc_evt_counter;

static uint8_t battery_level;   // last battery level measurement in percent

// RESULT = [V(P) � V(N) ] * GAIN/REFERENCE * 2(RESOLUTION - m)
// (m=0) if CONFIG.MODE=SE, or (m=1) if CONFIG.MODE=Diff.

// V(N) = 0;
// GAIN = 1/6;
// REFERENCE Voltage = internal (0.6V);
// RESOLUTION : 12 bit;
// m = 0;

// 12bit
// V(P) = RESULT x REFERENCE / ( GAIN x RESOLUTION) = RESULT x (600 / (1/6 x 2^(12)) =  ADC_RESULT x 0.87890625;
// multiply by 2 as the battery is in a 100k/100k voltage divider
#define ADC_RESULT_IN_MILLI_VOLTS(ADC_RESULT) ((ADC_RESULT * 0.87890625 * 2))

// conversion between volts to LiPo battery percent in 5% increments
static uint16_t lipo_table[20] = {4200, 4150, 4110, 4080, 4020, 3980, 3950, 3910, 3870, 3850,
								  3840, 3820, 3800, 3790, 3770, 3750, 3730, 3710, 3690, 3610 };

// convert millivolts to battery percent
static uint8_t calc_lipo_battery_level(uint16_t mvolts)
{
	for (int i = 0; i < 20; i++)
	{
		if (mvolts >= lipo_table[i])
		{
			return 100 - (i * 5);
		}
	}
	return 0;
}


static void timer_handler(nrf_timer_event_t event_type, void * p_context)
{

}


static void saadc_sampling_event_init(void)
{
    ret_code_t err_code;

    err_code = nrf_drv_ppi_init();
    APP_ERROR_CHECK(err_code);

    nrf_drv_timer_config_t timer_cfg = NRF_DRV_TIMER_DEFAULT_CONFIG;
    timer_cfg.bit_width = NRF_TIMER_BIT_WIDTH_32;
    err_code = nrf_drv_timer_init(&m_timer, &timer_cfg, timer_handler);
    APP_ERROR_CHECK(err_code);

    uint32_t ticks = nrf_drv_timer_ms_to_ticks(&m_timer, SAMPLE_PERIOD);
    nrf_drv_timer_extended_compare(&m_timer,
                                   NRF_TIMER_CC_CHANNEL0,
                                   ticks,
                                   NRF_TIMER_SHORT_COMPARE0_CLEAR_MASK,
                                   false);
    nrf_drv_timer_enable(&m_timer);

    uint32_t timer_compare_event_addr = nrf_drv_timer_compare_event_address_get(&m_timer,
                                                                                NRF_TIMER_CC_CHANNEL0);
    uint32_t saadc_sample_task_addr   = nrf_drv_saadc_sample_task_get();

    // setup ppi channel so that timer compare event is triggering sample task in SAADC
    err_code = nrf_drv_ppi_channel_alloc(&m_ppi_channel);
    APP_ERROR_CHECK(err_code);

    err_code = nrf_drv_ppi_channel_assign(m_ppi_channel,
                                          timer_compare_event_addr,
                                          saadc_sample_task_addr);
    APP_ERROR_CHECK(err_code);
}


static void saadc_sampling_event_enable(void)
{
    ret_code_t err_code = nrf_drv_ppi_channel_enable(m_ppi_channel);

    APP_ERROR_CHECK(err_code);
}


static void saadc_callback(nrf_drv_saadc_evt_t const * p_event)
{
    ret_code_t err_code;

    if (p_event->type == NRF_DRV_SAADC_EVT_DONE)
    {     
        uint32_t buffer_samples = 0;
        uint16_t adc_result = 0;
        uint16_t adc_voltage_mv = 0;

        err_code = nrf_drv_saadc_buffer_convert(p_event->data.done.p_buffer, SAMPLES_IN_BUFFER);
        APP_ERROR_CHECK(err_code);

        m_adc_evt_counter++;

        for (uint8_t i = 0; i < SAMPLES_IN_BUFFER; i++)
        {
            buffer_samples += p_event->data.done.p_buffer[i];
        }

        adc_result = buffer_samples/SAMPLES_IN_BUFFER;
        adc_voltage_mv = ADC_RESULT_IN_MILLI_VOLTS(adc_result);
		battery_level = calc_lipo_battery_level(adc_voltage_mv); 
    }
}


uint8_t get_battery_level(void)
{
	return battery_level;
}


static void saadc_init(void)
{
    ret_code_t err_code;
    // Using the channel default single ended (SE) config
    // pin NRF_SAADC_INPUT_AIN5 : P0.29
    nrf_saadc_channel_config_t channel_config =
        NRF_DRV_SAADC_DEFAULT_CHANNEL_CONFIG_SE(NRF_SAADC_INPUT_AIN5);

    // changing the saadc resolution to 12bit
    nrf_drv_saadc_config_t saadc_config ;
    saadc_config.resolution = NRF_SAADC_RESOLUTION_12BIT;

    err_code = nrf_drv_saadc_init(&saadc_config, saadc_callback);
    APP_ERROR_CHECK(err_code);

    err_code = nrf_drv_saadc_channel_init(0, &channel_config);
    APP_ERROR_CHECK(err_code);

    err_code = nrf_drv_saadc_buffer_convert(m_buffer_pool[0], SAMPLES_IN_BUFFER);
    APP_ERROR_CHECK(err_code);

    err_code = nrf_drv_saadc_buffer_convert(m_buffer_pool[1], SAMPLES_IN_BUFFER);
    APP_ERROR_CHECK(err_code);
}


void battery_init(void)
{
	saadc_init();
    saadc_sampling_event_init();
    saadc_sampling_event_enable();
}
