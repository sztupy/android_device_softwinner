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
 * This file implement acceleration sensor adapter for MXC622x.
 */

#define MXC622X_OFFSET_X		0
#define MXC622X_OFFSET_Y		0
#define MXC622X_OFFSET_Z		0
#define MXC622X_SENSITIVITY_X		64
#define MXC622X_SENSITIVITY_Y		64
#define MXC622X_SENSITIVITY_Z		64


#if SENSORS_ACC_DEBUG
#define LOGD(x...)		printf(x)
#define LOGE(x...)		printf(x)
#else
#define LOGD(x...)
#define LOGE(x...)
#endif
/**
 * NOTE:
 * You are required to get the correct install direction according 
 * the sensor placement on target board 
 */
#define MXC622X_INSTALL_DIR		6

#if (defined DEVICE_ACC_MXC622X)

#include <stdio.h>
#include <fcntl.h>
#include <sys/ioctl.h>

#define LOG_TAG "SensorACC"
#include <utils/Log.h>

#include <sensors_acc_mxc622x.h>

/* Use 'm' as magic number */
#define MXC622X_IOM			'm'

/* IOCTLs for MXC622X device */
#define MXC622X_IOC_PWRON		_IO (MXC622X_IOM, 0x00)
#define MXC622X_IOC_PWRDN		_IO (MXC622X_IOM, 0x01)
#define MXC622X_IOC_READXYZ		_IOR(MXC622X_IOM, 0x05, int[3])
#define MXC622X_IOC_READSTATUS		_IOR(MXC622X_IOM, 0x07, int[3])
#define MXC622X_IOC_SETDETECTION	_IOW(MXC622X_IOM, 0x08, unsigned char)

int acc_mxc622x_init(void)
{
	return 0;
}

int acc_mxc622x_open(void)
{
	int fd;
	fd = open("/dev/mxc622x", O_RDWR);
	if (fd < 0) {
		return -1;
	}

	return fd;
}

int acc_mxc622x_close(int fd)
{
	close(fd);

	return 0;
}

int acc_mxc622x_read_data(int fd, int *data)
{
	int tmp[3];

	int res = ioctl(fd, MXC622X_IOC_READXYZ, tmp);
	data[0] = (signed char)tmp[0];
	data[1] = (signed char)tmp[1];
	data[2] = MXC622X_OFFSET_Z;
	return res;
}

int acc_mxc622x_get_offset(int fd, int *offset_xyz)
{
	offset_xyz[0] = MXC622X_OFFSET_X;
	offset_xyz[1] = MXC622X_OFFSET_Y;
	offset_xyz[2] = MXC622X_OFFSET_Z;

	return 0;
}

int acc_mxc622x_set_new_offset(int fd, int *offset_xyz)
{
	return 0;
}

int acc_mxc622x_get_sensitivity(int fd, int *sensit_xyz)
{
	sensit_xyz[0] = MXC622X_SENSITIVITY_X;
	sensit_xyz[1] = MXC622X_SENSITIVITY_Y;
	sensit_xyz[2] = MXC622X_SENSITIVITY_Z;

	return 0;
}

int acc_mxc622x_get_install_dir(void)
{
	return MXC622X_INSTALL_DIR;
}

#endif /* DEVICE_ACC_MXC622X */

