/**
 * Copyright (c) 2024 Croxel, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
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
			.options = BT_LE_ADV_OPT_SCANNABLE,
			.interval_min = 0x0020,
			.interval_max = 0x0020,
			.peer = NULL,
		};

		err = bt_le_ext_adv_create(&adv_param, NULL, &advertising[i]);
		if (err)
		{
			LOG_ERR("Erro ao criar set %d (err %d)", i, err);
			return err;
		}

		struct bt_data ad_instancia[] = {
			BT_DATA(BT_DATA_NAME_COMPLETE, CONFIG_BT_DEVICE_NAME, sizeof(CONFIG_BT_DEVICE_NAME) - 1),
			BT_DATA(BT_DATA_MANUFACTURER_DATA, mfg_data_advertisers[i].mfg_data_adv_t, sizeof(mfg_data_advertisers[i].mfg_data_adv_t)),
			// BT_DATA_BYTES(BT_DATA_UUID16_ALL, 0x0D, 0x18),
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
		BT_DATA(BT_DATA_NAME_COMPLETE, CONFIG_BT_DEVICE_NAME, sizeof(CONFIG_BT_DEVICE_NAME) - 1),
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

int main(void)
{
	int err;

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
	}

	set_advertisement_equipament_id();

	for (;;)
	{

		k_sleep(K_SECONDS(1));
	}

	return 0;
}