/**
 * @file ble_services.c
 * @author Leandro Marinho
 * @brief 
 * @version 0.1
 * @date 2026-04-13
 * 
 * @copyright Copyright (c) 2026
 * 
 */

#include <zephyr/bluetooth/uuid.h>
#include <zephyr/bluetooth/gatt.h>
#include "main.h"
#include <stdint.h>
#include <zephyr/kernel.h>
#include <zephyr/bluetooth/conn.h>
#include <zephyr/sys/byteorder.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(ble_services);


static ssize_t read_first_name(struct bt_conn *conn,
							   const struct bt_gatt_attr *attr,
							   void *buf,
							   uint16_t len,
							   uint16_t offset)
{
	const char *name = get_first_name_metric();
	size_t name_len = strlen(name);
	LOG_INF("Reading first name: offset=%u, len=%u, name='%s', name_len=%zu", offset, len, name, name_len);
	ssize_t ret = bt_gatt_attr_read(conn, attr, buf, len, offset, name, name_len);
	LOG_INF("Read result: %zd", ret);
	return ret;
}

static ssize_t write_first_name(struct bt_conn *conn, const struct bt_gatt_attr *attr,
								const void *buf, uint16_t len, uint16_t offset, uint8_t flags)
{
	char first_name[14];
	if (offset + len > sizeof(first_name) - 1)
	{
		return BT_GATT_ERR(BT_ATT_ERR_INVALID_OFFSET);
	}
	memcpy(first_name + offset, buf, len);
	first_name[offset + len] = '\0';
	set_first_name_metric(first_name);

	return len;
}

BT_GATT_SERVICE_DEFINE(user_data_svc,
					   BT_GATT_PRIMARY_SERVICE(BT_UUID_UDS),
					   BT_GATT_CHARACTERISTIC(BT_UUID_GATT_FIRST_NAME,
											  BT_GATT_CHRC_READ | BT_GATT_CHRC_WRITE,
											  BT_GATT_PERM_READ | BT_GATT_PERM_WRITE,
											  read_first_name, write_first_name, NULL),);