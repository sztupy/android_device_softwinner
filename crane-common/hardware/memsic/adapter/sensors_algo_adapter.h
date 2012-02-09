/*****************************************************************************
 *  Copyright Statement:
 *  --------------------
 *  This software is protected by Copyright and the information and source code
 *  contained herein is confidential. The software including the source code
 *  may not be copied and the information contained herein may not be used or
 *  disclosed except with the written permission of MEMSIC Inc. (C) 2009
 *****************************************************************************/

/**
 * @file
 * @author  Robbie Cao<hjcao@memsic.cn>
 *
 * @brief
 * This file define eCompass algorithm adapter APIs.
 */

#ifndef __SENSORS_ALGO_ADAPTER_H__
#define __SENSORS_ALGO_ADAPTER_H__

#include <sensors_data_struct.h>
#include <sensors_algo_ids_util.h>

/**
 * @brief
 * eCompass algorithm
 */
struct algo_t {
	/**
	 * @brief Initialization first of all
	 */
	int (*init)(void);

	/**
	 * @brief Open eCompass algorithm
	 * @return 0 for success, others for failure
	 */
	int (*open)(void);
	/**
	 * @brief Close eCompass algorithm
	 * @return 0 for success, others for failure
	 */
	int (*close)(void);
	/**
	 * @brief Restart eCompass algorithm
	 * @return 0 for success, others for failure
	 */
	int (*restart)(void);
	/**
	 * @brief Get eCompass algorithm state
	 * @return the eCompass algorithm running state
	 */
	int (*get_state)(void);
	/**
	 * @brief Clear eCompass algorithm state
	 */
	void (*clear_state)(void);

	/**
	 * @brief Load eCompass algorithm parameters from NVM to RAM
	 * @return 0 for success, others for failure
	 */
	int (*nvm_load)(void);
	/**
	 * @brief Store eCompass algorithm parameters from RAM to NVM
	 * @return 0 for success, others for failure
	 */
	int (*nvm_store)(void);
	/**
	 * @brief Restore eCompass algorithm parameters on NVM to default
	 * @return 0 for success, others for failure
	 */
	int (*nvm_restore)(void);

	/**
	 * @brief Input sensor data to eCompass algorithm for calibration
	 * @param d contain data of acceleration sensor and magnetic sensor
	 */
	void (*data_in)(struct SensorData_Algo *d);
	/**
	 * @brief Input sensor data to eCompass algorithm to calculate 
	 *        current orientation(heading/pitch/roll)
	 * @param o is the calculating result
	 * @param d contain data of acceleration sensor and magnetic sensor
	 * @return 0 for success, others for failure
	 */
	int (*calc_orientation)(struct SensorData_Orientation *o, struct SensorData_Algo *d);
	/**
	 * @brief Calibrate magnetic sensor
	 * @param mag[3] contain data of magnetic sensor
	 * @param centre[3] contain center of magnetic sensor to return
	 * @return 0 for success, others for failure
	 */
	int (*calc_magcentre)(const float mag[3], float centre[3]);
	/**
	 * @brief Calculate calibrated magnetic data
	 * @param d contain data of magnetic sensor
	 * @param cald[3] contain calibrated magnetic data to return
	 * @return 0 for success, others for failure
	 */
	int (*calc_magcal_data)(struct SensorData_Algo *d, float cald[3]);
	/**
	 * @brief Calibrate eCompass
	 * @param d contain data of acceleration sensor and magnetic sensor
	 * @return 0 for success, others for failure
	 */
	int (*calibrate)(struct SensorData_Algo *d);
};

/**
 * @brief Get eCompass algorithm
 * @return the instance of eCompass algorithm
 */
struct algo_t *sensors_get_algorithm(void);

#endif /* __SENSORS_ALGO_ADAPTER_H__ */
