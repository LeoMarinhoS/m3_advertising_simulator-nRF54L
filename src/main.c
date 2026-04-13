/**
 * @file main.c

 * @author Leandro Marinho (robotdancepopping@gmail.com)
 * @brief 
 * @version 0.1
 * @date 2026-04-13
 * 
 * @details This code implements a Bluetooth Low Energy (BLE) peripheral advertiser using
 *  the Zephyr RTOS. It creates multiple advertising sets, each with its own manufacturer
 *  data, and updates the advertised cadence and power values dynamically every second.
 *  The advertiser simulates a fitness equipment broadcasting its status, including cadence,
 *  power, and other relevant metrics.
 * 
 * @copyright Copyright (c) 2026
 * 
 */

#include <zephyr/bluetooth/bluetooth.h>
#include <zephyr/bluetooth/conn.h>
#include <zephyr/bluetooth/gap.h>
#include <zephyr/bluetooth/hci.h>
#include <zephyr/logging/log.h>
#include <zephyr/bluetooth/conn.h>
#include <zephyr/kernel.h>
#include "main.h"

LOG_MODULE_REGISTER(main_ble_manager);

static char instances_names_seted[14] = CONFIG_BT_DEVICE_NAME; /*< Array to store the names of the advertising instances >*/

struct bt_le_ext_adv *advertising[CONFIG_BT_EXT_ADV_MAX_ADV_SET];

ble_advertisements_t mfg_data_advertisers[CONFIG_BT_EXT_ADV_MAX_ADV_SET] = {
	[0 ...(CONFIG_BT_EXT_ADV_MAX_ADV_SET - 1)] = {
		.mfg_data_adv_t = {
			[COMPANY_ID_MSB] = 0x02,
			[COMPANY_ID_LSB] = 0x01,
			[VERSION_MAJOR] = 0x06,
			[VERSION_MINOR] = 0x61,
			[DATA_TYPE] = 0x00,
			[EQUIPAMENT_ID] = 0x01,
			[CADENCE] = 0x26,
			[CADENCE_BYTES] = 0x00,
			[HEART_RATE] = 0x48,
			[HEAT_RATE_BYTES] = 0x00,
			[POWER] = 0x1E,
			[POWER_BYTES] = 0x00,
			[CALORIC_BURN] = 0x0d,
			[CALORIC_BURN_BYTES] = 0x00,
			[DURATION_MINUTES] = 0x00,
			[DURATION_SECONDS] = 0x3C,
			[DISTANCE] = 0x00,
			[DISTANCE_BYTES] = 0x00,
			[GEAR] = 0x0A
		}}}; /*< Manufacturer data for the advertiser */

static struct bt_conn *conectable;/*<! Global pointer for saving the connection.*/
static void restart_advertising(struct k_work *work);

static K_WORK_DEFINE(restart_advertising_worker, restart_advertising);

static void restart_advertising(struct k_work *work)
{
    for (int i = 0; i < CONFIG_BT_EXT_ADV_MAX_ADV_SET; i++) {
        if (advertising[i] == NULL) continue;
        int err = bt_le_ext_adv_start(advertising[i], BT_LE_EXT_ADV_START_DEFAULT);
        if (err) {
            LOG_ERR("Falha ao reiniciar instância %d (err %d)", i, err);
        }
    }
}

static void connected(struct bt_conn *conn, uint8_t err)
{
    if (err) {
        LOG_ERR("Connection failed (err %u)", err);
        return;
    }

	LOG_INF("Connected.");
    conectable = bt_conn_ref(conn);

    for (int i = 0; i < CONFIG_BT_EXT_ADV_MAX_ADV_SET; i++) {
        if (advertising[i] != NULL) {
            bt_le_ext_adv_stop(advertising[i]);
        }
    }
}

static void disconnected(struct bt_conn *conn, uint8_t reason)
{
    LOG_WRN("Disconnected (reason 0x%02x)", reason);

    if (conectable) {
        bt_conn_unref(conectable);
        conectable = NULL;
    }
    k_work_submit(&restart_advertising_worker);
}

static void recycled_cb(void)
{
    k_work_submit(&restart_advertising_worker);
}

BT_CONN_CB_DEFINE(conn_callbacks) = {
    .connected    = connected,
    .disconnected = disconnected,
    .recycled     = recycled_cb,
};

int start_all_advertisers(void)
{
	int err;

	for (int i = 0; i < CONFIG_BT_EXT_ADV_MAX_ADV_SET; i++)
	{
		mfg_data_advertisers[i].mfg_data_adv_t[EQUIPAMENT_ID] = (uint8_t)(i + 1);

		struct bt_le_adv_param adv_param = {
			.id = BT_ID_DEFAULT,
			.sid = (uint8_t)i,
			.secondary_max_skip = 0U,
			.options = (i == 0) ? (BT_LE_ADV_OPT_CONNECTABLE) : BT_LE_ADV_OPT_SCANNABLE, /*< First advertiser is connectable with name, others are non-connectable */
			.interval_min = BT_GAP_ADV_FAST_INT_MIN_1,
			.interval_max = BT_GAP_ADV_FAST_INT_MAX_1,
			.peer = NULL,
		};

		err = bt_le_ext_adv_create(&adv_param, NULL, &advertising[i]);
		if (err)
		{
			LOG_ERR("Erro ao criar set %d (err %d)", i, err);
			return err;
		}

		struct bt_data ad_instancia[] = {
			BT_DATA(BT_DATA_NAME_COMPLETE, instances_names_seted, strlen(instances_names_seted)),
			BT_DATA(BT_DATA_MANUFACTURER_DATA, mfg_data_advertisers[i].mfg_data_adv_t, sizeof(mfg_data_advertisers[i].mfg_data_adv_t)),
		};

		err = bt_le_ext_adv_set_data(advertising[i], ad_instancia, ARRAY_SIZE(ad_instancia), NULL, 0);
		if (err)
			return err;

		err = bt_le_ext_adv_start(advertising[i], BT_LE_EXT_ADV_START_DEFAULT);
		if (err)
			return err;

		LOG_INF("Instância %d iniciada (Equip ID: %d)", i, i + 1);
	}

	return 0;
}

void update_advertisement_ble(uint8_t index, ble_advertisements_t data)
{
	if (index >= CONFIG_BT_EXT_ADV_MAX_ADV_SET)
		return;

	int err;
	memcpy(mfg_data_advertisers[index].mfg_data_adv_t, data.mfg_data_adv_t, sizeof(mfg_data_advertisers[index].mfg_data_adv_t));

	struct bt_data ad_update[] = {
		BT_DATA(BT_DATA_NAME_COMPLETE, instances_names_seted, strlen(instances_names_seted)),
		BT_DATA(BT_DATA_MANUFACTURER_DATA, mfg_data_advertisers[index].mfg_data_adv_t, sizeof(mfg_data_advertisers[index].mfg_data_adv_t)),
		BT_DATA_BYTES(BT_DATA_UUID16_ALL, 0x0D, 0x18),
	};

	err = bt_le_ext_adv_set_data(advertising[index], ad_update, ARRAY_SIZE(ad_update), NULL, 0);
	if (err)
	{
		LOG_ERR("Falha na atualização da instância %d", index);
	}
}

void set_advertisement_equipament_id()
{
	for(uint8_t index = 0; index < CONFIG_BT_EXT_ADV_MAX_ADV_SET; index++)
	{
		mfg_data_advertisers[index].mfg_data_adv_t[EQUIPAMENT_ID] = index + 1;
		update_advertisement_ble(index, mfg_data_advertisers[index]);
	}
}

void set_advertisement_equipament_cadence(uint8_t index, uint8_t cadence)
{
	if (index >= CONFIG_BT_EXT_ADV_MAX_ADV_SET)
		return;

	mfg_data_advertisers[index].mfg_data_adv_t[CADENCE] = cadence;
	update_advertisement_ble(index, mfg_data_advertisers[index]);
}

void set_advertsisement_power(uint8_t index, uint8_t power)
{
	if (index >= CONFIG_BT_EXT_ADV_MAX_ADV_SET)
		return;

	mfg_data_advertisers[index].mfg_data_adv_t[POWER] = power;
	update_advertisement_ble(index, mfg_data_advertisers[index]);
}

void set_advertisement_dynamic_values(uint8_t index, uint16_t cadence, uint16_t power)
{
	if (index >= CONFIG_BT_EXT_ADV_MAX_ADV_SET)
		return;

	mfg_data_advertisers[index].mfg_data_adv_t[CADENCE] = (uint8_t)cadence;
	mfg_data_advertisers[index].mfg_data_adv_t[CADENCE_BYTES] = (uint8_t)(cadence >> 8);
	mfg_data_advertisers[index].mfg_data_adv_t[POWER] = (uint8_t)power;
	mfg_data_advertisers[index].mfg_data_adv_t[POWER_BYTES] = (uint8_t)(power >> 8);
	update_advertisement_ble(index, mfg_data_advertisers[index]);
}

static void update_all_advertisers_dynamic(uint16_t cadence, uint16_t power)
{
	for (uint8_t index = 0; index < CONFIG_BT_EXT_ADV_MAX_ADV_SET; index++)
	{
		set_advertisement_dynamic_values(index, cadence, power);
	}
}

const char* get_first_name_metric(void)
{
	return instances_names_seted;
}

void set_first_name_metric(const char * value)
{
	strncpy(instances_names_seted, value, sizeof(instances_names_seted) - 1);
	instances_names_seted[sizeof(instances_names_seted) - 1] = '\0'; // Garantir terminação nula
	for (uint8_t index = 0; index < CONFIG_BT_EXT_ADV_MAX_ADV_SET; index++)
	{
		update_advertisement_ble(index, mfg_data_advertisers[index]);
	}
} 

int main(void)
{
	int err;
	uint16_t cadence = CADENCE_MIN;
	uint16_t power = POWER_MIN;

	LOG_INF("Starting Bluetooth Peripheral Advertiser example");

	err = bt_enable(NULL);
	if (err)
	{
		LOG_ERR("Bluetooth init failed (err %d)", err);
		return err;
	}

	LOG_INF("Bluetooth initialized");

	err = start_all_advertisers();
	if (err)
	{
		LOG_ERR("Failed to start advertising (err %d)", err);
		return err;
	}

	set_advertisement_equipament_id(); /*< Initialize equipment IDs */
	update_all_advertisers_dynamic(cadence, power); /*< Update all advertisers with initial values */

	for (;;)
	{
		k_sleep(K_SECONDS(1));

		if (conectable) { /*< Do nothing if connected */
			continue;
		}

		if (power < POWER_MAX) {
			power += 4;
			if (power >= POWER_MAX) {
				power = POWER_MIN;
			}
		}

		if (cadence < CADENCE_MAX) {
			cadence++;
		}

		/*Reset values*/
		cadence = (cadence >= CADENCE_MAX) ? CADENCE_MIN : cadence;

		update_all_advertisers_dynamic(cadence, power);
		// LOG_INF("Updated cadence=%u RPM power=%u W", cadence, power);
	}

	return 0;
}