/**
 * @file main.h
 * @author Leandro Marinho
 * @brief 
 * @version 0.1
 * @date 2026-04-10
 * 
 * @copyright Copyright (c) 2026
 * 
 */


#ifndef MAIN_H
#define MAIN_H

#include <zephyr/bluetooth/bluetooth.h>
#include <zephyr/bluetooth/conn.h>
#include <zephyr/bluetooth/gap.h>
#include <zephyr/bluetooth/hci.h>

/* Manufacturer data structure for the advertiser */
typedef enum
{
    COMPANY_ID_MSB,   /*< Most Significant Byte of Company ID */
    COMPANY_ID_LSB,   /*< Least Significant Byte of Company ID */
    VERSION_MAJOR,    /*< Major version number */
    VERSION_MINOR,    /*< Minor version number */
    DATA_TYPE,        /*< Data type */
    EQUIPAMENT_ID,    /*< Equipment ID */
    CADENCE,          /*< Cadence */
    CADENCE_BYTES,    /*< Cadence in bytes (2 bytes) */
    HEART_RATE,       /*< Heart rate */
    HEAT_RATE_BYTES,   /*< Heart rate in bytes (2 bytes) */
    POWER,            /*< Power */
    POWER_BYTES,      /*< Power in bytes (2 bytes) */
    CALORIC_BURN,     /*< Caloric burn */
    CALORIC_BURN_BYTES, /*< Caloric burn in bytes (2 bytes) */
    DURATION_MINUTES, /*< Duration in minutes */
    DURATION_SECONDS, /*< Duration in seconds */
    DISTANCE,         /*< Distance */
    DISTANCE_BYTES,   /*< Distance in bytes (2 bytes) */
    GEAR,

    ID_COUNT /*< Number of advertisement bytes*/

} advertisements_id_t;

typedef struct ble_advertisements_t {

    uint8_t mfg_data_adv_t[ID_COUNT];

}ble_advertisements_t;

#define POWER_MIN 60
#define POWER_MAX 300

#define CADENCE_MIN 6
#define CADENCE_MAX 120

#define APPLICATION_VERSION "0.1"

const char* get_first_name_metric(void);
void set_first_name_metric(const char * value);



#endif /* MAIN_H */