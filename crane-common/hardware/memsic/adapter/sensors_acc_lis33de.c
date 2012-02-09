/*****************************************************************************
 *  Copyright Statement:
 *  --------------------
 *  This software is protected by Copyright and the information and source code
 *  contained herein is confidential. The software including the source code
 *  may not be copied and the information contained herein may not be used or
 *  disclosed except with the written permission of ZTE Inc. (C) 2010
 *****************************************************************************/

/**
 * @file
 * @author  Zhiyuan Li<li.zhiyuan1@zte.com.n>
 *
 * @brief
 * This file implement acceleration sensor adapter for LIS33DE.
 */
#if (defined DEVICE_ACC_LIS33DE)

#define LOG_TAG "SensorLIS33DE"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/ipc.h>
#include <sys/mman.h>

#include <sensors_acc_lis33de.h>

#ifdef MODULE_DEBUG
#include <utils/Log.h>
#endif

#define SENSORS_ACC_DEBUG	1

#if (!defined MODULE_DEBUG)
#if SENSORS_ACC_DEBUG
#define LOGD(x...)		printf(x)
#define LOGE(x...)		printf(x)
#else
#define LOGD(x...)
#define LOGE(x...)
#endif
#endif

#define LIS33DE_OFFSET_X			0
#define LIS33DE_OFFSET_Y			0
#define LIS33DE_OFFSET_Z			0
#define LIS33DE_SENSITIVITY_X		56
#define LIS33DE_SENSITIVITY_Y		56
#define LIS33DE_SENSITIVITY_Z		56

/**
 * NOTE:
 * You are required to get the correct install direction according 
 * the sensor placement on target board 
 */
#define LIS33DE_INSTALL_DIR		3

/*----------------------------------------------------------*/
// include header for sensor LIS33DE here
/*----------------------------------------------------------*/

/*----------------------------------------------------------*/
#define HWM_IOC_MAGIC           0x91

/* IOCTLs for LIS33DE device */
#define HWM_IOCG_ACC_RAW_lis33de        _IOR(HWM_IOC_MAGIC, 0x19,signed char[3] )   /*!< get raw data*/
/*----------------------------------------------------------*/





int acc_lis33de_init(void)
{
	return 0;
}

int acc_lis33de_open(void)
{
	int fd;

	fd = open("/dev/lis33de", O_RDWR);
	if (fd < 0) {
		return -1;
	}

	return fd;
}

int acc_lis33de_close(int fd)
{
	close(fd);
	return 0;
}

int acc_lis33de_read_data(int fd, int *data)
{
	signed char acc[3] = {0};

	assert(data != NULL);


	if (ioctl(fd, HWM_IOCG_ACC_RAW_lis33de, acc)) {
		LOGE("LIS33DE_READ_ACCEL_XYZ fail");
		return -1;
	}
	*data = acc[0];
	*(data+1) =  acc[1];
	*(data+2) =  acc[2];
	return 0;
}

int acc_lis33de_get_offset(int fd, int *offset_xyz)
{
	offset_xyz[0] = LIS33DE_OFFSET_X;
	offset_xyz[1] = LIS33DE_OFFSET_Y;
	offset_xyz[2] = LIS33DE_OFFSET_Z;
	return 0;
}
#if 0
int acc_lis33de_set_new_offset(int fd, int *offset_xyz)
{
	
	return 0;
}
#endif 

int acc_lis33de_get_sensitivity(int fd, int *sensit_xyz)
{
	// FIXME:
	// should get sensivitity from vendor sensor config or from sensor device
	// const value '18' for LIS33DE tmp
	assert(sensit_xyz != NULL);

	sensit_xyz[0] = LIS33DE_SENSITIVITY_X;
	sensit_xyz[1] = LIS33DE_SENSITIVITY_Y;
	sensit_xyz[2] = LIS33DE_SENSITIVITY_Z;

	return 0;
}

int acc_lis33de_get_install_dir(void)
{
	return LIS33DE_INSTALL_DIR;
}

#endif /* DEVICE_ACC_LIS33DE */

