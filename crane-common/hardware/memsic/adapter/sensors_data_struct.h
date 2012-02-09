/*****************************************************************************
 *  Copyright Statement:
 *  --------------------
 *  This software is protected by Copyright and the information and source code
 *  contained herein is confidential. The software including the source code
 *  may not be copied and the information contained herein may not be used or
 *  disclosed except with the written permission of MEMSIC Inc. (C) 2009
 *****************************************************************************/

/**
 * @mainpage MEMSIC Sensor Solution for Android
 *
 * @section Introduction
 * MEMSIC sensor solution for Android provides total solution include:
 * - Sensor Device Driver
 * - Daemon
 * - Hardware Abstract Layer(HAL) for Android
 * This document will cover all the modules except device driver.
 * 
 * @section Daemon
 * This module integrate MEMSIC 3-axis magnetic sensor combine with 
 * 2/3-axis acceleration sensor from major g-sensor vendor.
 * @subsection Adapters
 * MEMSIC sensor solution has several adapters to integrate different 
 * accelerometer, magnetic sensor and eCompass algorithm from different 
 * device & algorithm vendors, Include:
 * - Acceleration sensor adapter
 * - Magnetic sensor adapter
 * - eCompass algorithm adapter
 * @subsection Integrator
 * Integrator will poll raw data from sensor device and feed the data 
 * into proper algorithm to calculate yaw/pitch/roll values. 
 * This part will report yaw/pitch/roll, acceleration value or magnetic 
 * field value to sensor HAL module through Linux input sub-system.
 * 
 * @section HAL
 * This module implement sensor hardware abstract layer for Android. 
 * It’s an implementation of the APIs defined in ‘sensors.h’ under 
 * Android source tree.
 */


/**
 * @file
 * @author  Robbie Cao<hjcao@memsic.cn>
 *
 * @brief
 * This file define basic sensor data structures
 */

#ifndef __SENSORS_DATA_STRUCT_H__
#define __SENSORS_DATA_STRUCT_H__

#include <stdio.h>
__BEGIN_DECLS
#if (defined COMPASS_ALGO_H5)
#include "CompassLib_H5.h"
#elif (defined COMPASS_ALGO_H6)
#include "CompassLib_H6.h"
#else
#error "Algorithm not specify!"
#endif
__END_DECLS

/**
 * @brief
 * Sensor raw data collection
 */
struct SensorData_Raw {
	int acc[3];	///< acceleration raw vector
	int off_a[3];	///< acceleration offset vector
	int sens_a[3];	///< acceleration sensitivity vector
	int dir_a;	///< acc sensor placement on target board

	int mag[3];	///< magnetic raw vector
	int off_m[3];	///< magnetic offset vector
	int sens_m[3];	///< magnetic sensitivity vector
	int dir_m;	///< mag sensor placement on target board
};

/**
 * @brief
 * Sensor data in real physical unit
 */
struct SensorData_Real {
	float acc[3];	///< acceleration
	float mag[3];	///< magnetic
};

/**
 * @brief
 * Sensor data to feed in algorithm
 */
struct SensorData_Algo {
	uint16 hx;	///< magnetic x-axis
	uint16 hy;	///< magnetic y-axis
	uint16 hz;	///< magnetic z-axis
	int16 gx;	///< acceleration x-axis
	int16 gy;	///< acceleration y-axis
	int16 gz;	///< acceleration z-axis
};

/**
 * @brief
 * Orientation result
 */
struct SensorData_Orientation {
	int azimuth;	///< heading value in degree
	int pitch;	///< pitch value in degree
	int roll;	///< roll value in degree

	int noise_level;	///< noise level
	int quality;		///< quality level in percent
};

#define ACC_UNIFY(_r, _o, _s)	(((_r) - (_o)) / ((float)(_s)))
#define MAG_UNIFY(_r, _o, _s)	(((_r) - (_o)) / ((float)(_s)))

#define ACC_NORM(_r, _o, _s)	(((_r) - (_o)) * 32768 / (_s))
#define MAG_NORM(_r, _o, _s)	(((_r) - (_o)) * 32768 / (_s))

#define ACC_NORM2(_r)		((_r) * 32768)
#define MAG_NORM2(_r)		((_r) * 32768)

#endif /* __SENSORS_DATA_STRUCT_H__ */
