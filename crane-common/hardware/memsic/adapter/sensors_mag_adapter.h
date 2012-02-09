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
 * This file define magnetic sensor adapter APIs.
 */

#ifndef __SENSORS_MAG_ADAPTER_H__
#define __SENSORS_MAG_ADAPTER_H__

/**
 * @brief
 * Magnetic sensor device
 */
struct device_mag_t {
	/**
	 * @brief Initialization first of all
	 */
	int (*init)(void);

	/**
	 * @brief Open device
	 * @return fd of magnetic sensor device, -1 for failure
	 */
	int (*open)(void);
	/**
	 * @brief Close device
	 * @param fd is the file descriptor of sensor device
	 * @return 0 for success, others for failure
	 */
	int (*close)(int fd);

	/**
	 * @brief Get sensor raw data from sensor device
	 * @param fd is the file descriptor of sensor device
	 * @param data is the raw data vector
	 * @return 0 for success, others for failure
	 */
	int (*read_data)(int fd, int *data);
	/**
	 * @brief Get sensor offset from sensor device
	 * @param fd is the file descriptor of sensor device
	 * @param offset_xyz is the offset vector
	 * @return 0 for success, others for failure
	 */
	int (*get_offset)(int fd, int *offset_xyz);
	/**
	 * @brief Get sensor sensitivity from sensor device
	 * @param fd is the file descriptor of sensor device
	 * @param sensit_xyz is the sensitivity vector
	 * @return 0 for success, others for failure
	 */
	int (*get_sensitivity)(int fd, int *sensit_xyz);
	/**
	 * @brief Get sensor placement on target board
	 * @return sensor placement defined in sensors_placement_t
	 */
	int (*get_install_dir)(void);
};

/**
 * @brief Get magnetic sensor device
 * @return the instance of magnetic sensor device
 */
struct device_mag_t *sensors_get_mag_device(void);

#endif /* __SENSORS_MAG_ADAPTER_H__ */

