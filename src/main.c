/*safety band GNSS code*/
#include <stdio.h>
#include <ncs_version.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <modem/nrf_modem_lib.h>
#include <modem/lte_lc.h>
#include <dk_buttons_and_leds.h>
#include <nrf_modem_gnss.h>

LOG_MODULE_REGISTER(GNSS, LOG_LEVEL_INF);
static struct nrf_modem_gnss_pvt_data_frame pvt_data;
static int64_t gnss_start_time;
static bool first_fix = false;
static K_SEM_DEFINE(lte_connected, 0, 1);

static int modem_configure(void)
{
	int err;

	LOG_INF("Initializing modem library");

	err = nrf_modem_lib_init();
	if (err)
	{
		LOG_ERR("Failed to initialize the modem library, error: %d", err);
		return err;
	}

	return 0;
}

static void print_fix_data(struct nrf_modem_gnss_pvt_data_frame *pvt_data)
{
	LOG_INF("Latitude:       %.06f", pvt_data->latitude);
	LOG_INF("Longitude:      %.06f", pvt_data->longitude);
	LOG_INF("Altitude:       %.01f m", (double)pvt_data->altitude);
	LOG_INF("Time (UTC):     %02u:%02u:%02u.%03u",
			pvt_data->datetime.hour,
			pvt_data->datetime.minute,
			pvt_data->datetime.seconds,
			pvt_data->datetime.ms);
}

static void gnss_event_handler(int event)
{
	int err;

	switch (event)
	{
	case NRF_MODEM_GNSS_EVT_PVT:
		LOG_INF("Searching...");
		int num_satellites = 0;
		// for (int i = 0; i < 12; i++)
		for (int i = 0; i < NRF_MODEM_GNSS_MAX_SATELLITES; i++)
		{
			if (pvt_data.sv[i].signal != 0)
			{
				LOG_INF("sv: %d, cn0: %d, signal: %d", pvt_data.sv[i].sv, pvt_data.sv[i].cn0, pvt_data.sv[i].signal);
				num_satellites++;
			}
		}

		LOG_INF("Number of current satellites: %d", num_satellites);
		err = nrf_modem_gnss_read(&pvt_data, sizeof(pvt_data), NRF_MODEM_GNSS_DATA_PVT);
		if (err)
		{
			LOG_ERR("nrf_modem_gnss_read failed, err %d", err);
			return;
		}

		if (pvt_data.flags & NRF_MODEM_GNSS_PVT_FLAG_FIX_VALID)
		{
			int satellites_used = 0;

			for (int i = 0; i < NRF_MODEM_GNSS_MAX_SATELLITES; i++)
			{
				if (pvt_data.sv[i].flags & NRF_MODEM_GNSS_SV_FLAG_USED_IN_FIX)
				{
					satellites_used++;
				}
			}

			if (satellites_used == 3)
			{
				LOG_INF("2D Fix obtained!");
				LOG_INF("Latitude: %.06f", pvt_data.latitude);
				LOG_INF("Longitude: %.06f", pvt_data.longitude);
			}
			else if (satellites_used >= 4)
			{
				LOG_INF("3D Fix obtained!");
				print_fix_data(&pvt_data); // shows latitude, longitude, altitude
			}

			dk_set_led_on(DK_LED1);
			
			if (!first_fix)
			{
				LOG_INF("Time to first fix: %2.1lld s", (k_uptime_get() - gnss_start_time) / 1000);
				first_fix = true;
			}
			return;
		}
		break;
	case NRF_MODEM_GNSS_EVT_PERIODIC_WAKEUP:
		LOG_INF("GNSS has woken up");
		break;
	case NRF_MODEM_GNSS_EVT_SLEEP_AFTER_FIX:
		LOG_INF("GNSS enter sleep after fix");
		break;
	default:
		break;
	}
}

int main(void)
{
	int err;

	if (dk_leds_init() != 0)
	{
		LOG_ERR("Failed to initialize the LEDs Library");
	}

	err = modem_configure();
	if (err)
	{
		LOG_ERR("Failed to configure the modem");
		return 0;
	}
// -----------------------------------------
	uint8_t use_case;

	use_case = NRF_MODEM_GNSS_USE_CASE_MULTIPLE_HOT_START | NRF_MODEM_GNSS_USE_CASE_LOW_ACCURACY;

	err = nrf_modem_gnss_use_case_set(use_case);
	if (err != 0)
	{
		printk("Failed to set use case error= %d ", err);
	}

	/*Activate Normal Mode GNSS + BLE + LTE */
	if (lte_lc_func_mode_set(LTE_LC_FUNC_MODE_NORMAL) != 0)
	{
		LOG_ERR("Failed to activate GNSS functional mode");
		return 0;
	}
	if (nrf_modem_gnss_event_handler_set(gnss_event_handler) != 0)
	{
		LOG_ERR("Failed to set GNSS event handler");
		return 0;
	}

	/* Continious scanning*/
	if (nrf_modem_gnss_fix_interval_set(1) != 0)
	{
		LOG_ERR("Failed to set GNSS fix interval");
		return 0;
	}

	if (nrf_modem_gnss_fix_retry_set(0) != 0)
	{
		LOG_ERR("Failed to set GNSS fix retry");
		return 0;
	}
	LOG_INF("Starting GNSS Continous scanning");
	
	/* Start the GNSS receiver*/
	if (nrf_modem_gnss_start() != 0)
	{
		LOG_ERR("Failed to start GNSS");
		return 0;
	}
	gnss_start_time = k_uptime_get();

	return 0;
}
