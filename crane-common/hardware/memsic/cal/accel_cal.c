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
 * This file implement acceleration sensor offset calibration module.
 */

#define LOG_TAG "SensorCal"

#include <stdio.h>
#include <stdlib.h>
#include <dlfcn.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/ioctl.h>

#include <cutils/atomic.h>
#include <utils/Log.h>

#include <sensors_acc_adapter.h>
#include <sensors_algo_adapter.h>


#define ACCEL_NUM_AVG			10
#define ACCEL_TRIES_MAX			20

/**
 * @brief
 * Acceleration data vector
 */
typedef struct  {
	short x;	///< acceleration x value
	short y;	///< acceleration y value
	short z;	///< acceleration z value
} acc_t;

static struct algo_t *algo = NULL;
static struct device_acc_t *dev_acc = NULL;
static int fd_acc = -1;
static int ACCEL_MIN_MAX_THRESHOLD = 10;
/**
 * @brief Reads out acceleration data and averages them, measures min and max
 * @param num_avg numer of samples for averaging
 * @param *min returns the minimum measured value
 * @param *max returns the maximum measured value
 * @param *avg returns the average value
 * @return 0 for success, others for failure
 */
static int read_accel_avg(int num_avg, acc_t *min, acc_t *max, acc_t *avg)
{
	long x_avg = 0, y_avg = 0, z_avg = 0;
	int comres = 0;
	int i;
	acc_t accel;		/* read accel data */
	int raw_acc[3];

	x_avg = 0; y_avg = 0; z_avg = 0;                  
	max->x = -512; max->y = -512; max->z = -512;
	min->x = 512; min->y = 512; min->z = 512;
     

	for (i = 0; i < num_avg; i++) {
		/* read num_avg acceleration data triples */
		comres += dev_acc->read_data(fd_acc, raw_acc);
		accel.x = raw_acc[0];
		accel.y = raw_acc[1];
		accel.z = raw_acc[2];

		if (accel.x > max->x)
			max->x = accel.x;
		if (accel.x < min->x) 
			min->x=accel.x;

		if (accel.y > max->y)
			max->y = accel.y;
		if (accel.y < min->y) 
			min->y=accel.y;

		if (accel.z > max->z)
			max->z = accel.z;
		if (accel.z < min->z) 
			min->z=accel.z;
		
		x_avg += accel.x;
		y_avg += accel.y;
		z_avg += accel.z;
	}
	/* calculate averages, min and max values */
	avg->x = x_avg /= num_avg;
	avg->y = y_avg /= num_avg;
	avg->z = z_avg /= num_avg;

	return comres;
}
	 

/**
 * @brief Verifies the accerleration values to be good enough for calibration calculations
 * @param min takes the minimum measured value
 * @param max takes the maximum measured value
 * @param takes returns the average value
 * @return 1 for min,max values are in range, 0 for not in range
*/
static int verify_min_max(acc_t min, acc_t max, acc_t avg)
{
	short dx, dy, dz;
	int ver_ok = 1;
	int sensitivity_for_threshold[3];
	
	/* calc delta max-min */
	dx =  max.x - min.x;
	dy =  max.y - min.y;
	dz =  max.z - min.z;

	dev_acc->get_sensitivity(fd_acc, sensitivity_for_threshold);
	ACCEL_MIN_MAX_THRESHOLD = ( sensitivity_for_threshold[0] / 60 ) + 2;

	if (dx > ACCEL_MIN_MAX_THRESHOLD || dx < -ACCEL_MIN_MAX_THRESHOLD)
		ver_ok = 0;
	if (dy > ACCEL_MIN_MAX_THRESHOLD || dy < -ACCEL_MIN_MAX_THRESHOLD)
		ver_ok = 0;
	if (dz > ACCEL_MIN_MAX_THRESHOLD || dz <- ACCEL_MIN_MAX_THRESHOLD)
		ver_ok = 0;

	return ver_ok;
}	


/**
 * @brief Overall calibration process. <br>
 *        This function takes care about all other functions 
 * @return 0 for calibration failed, 1 for calibration passed
*/
int accel_calibrate(void)
{
	int min_max_ok = 0;
	int tries = ACCEL_TRIES_MAX;
	int res = 0;
	int val[4];
	int sensit_xyz[3];
	int install_dir = 0;

	acc_t min, max, avg;

	LOGD("[accel_calibrate]");

	dev_acc = sensors_get_acc_device();
	dev_acc->init();
	fd_acc = dev_acc->open();
	if (fd_acc < 0) {
		LOGE("open acc dev failed");
		return -1;
	}

	algo = sensors_get_algorithm();
	algo->init();
	algo->open();

	do {
		/* read acceleration data min, max, avg */
		res = read_accel_avg(ACCEL_NUM_AVG, &min, &max, &avg);
		if (res)	// there's failure in read acc data
			break;

		min_max_ok = verify_min_max(min, max, avg);

		if (tries <= 0)	/*number of maximum tries reached? */
			break;

		tries--;
	} while (!min_max_ok);

	if (!min_max_ok) {	// calibration fail
		dev_acc->close(fd_acc);
		algo->close();
		return 0;
	}

	dev_acc->get_sensitivity(fd_acc, sensit_xyz);
	install_dir = dev_acc->get_install_dir();
	if (install_dir >= 0 && install_dir <= 3) {	// sensor on board top side
		val[0] = avg.x;
		val[1] = avg.y;
		val[2] = (avg.z - sensit_xyz[2]);
	} else {		// sensor on board bottom side
		val[0] = avg.x;
		val[1] = avg.y;
		val[2] = (avg.z + sensit_xyz[2]);
	}
	val[3] = min_max_ok;	// status

	// set new offset and save it into FS
	dev_acc->set_new_offset(fd_acc, val);
	// restore default ecompass parm into non-volatile storage
	algo->nvm_restore();

	dev_acc->close(fd_acc);
	algo->close();

	return min_max_ok;
}

