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
 * This file implement acceleration sensor adapter for MXC6202x.
 */

#if ((defined DEVICE_ACC_MXC6202XG) ||	\
     (defined DEVICE_ACC_MXC6202XH) ||	\
     (defined DEVICE_ACC_MXC6202XM) ||	\
     (defined DEVICE_ACC_MXC6202XN))
#define DEVICE_ACC_MXC6202X
#endif

#if (defined DEVICE_ACC_MXC6202XG)
#define MXC6202X_OFFSET_X		2048
#define MXC6202X_OFFSET_Y		2048
#define MXC6202X_OFFSET_Z		2048
#define MXC6202X_SENSITIVITY_X		512
#define MXC6202X_SENSITIVITY_Y		512
#define MXC6202X_SENSITIVITY_Z		512
#elif (defined DEVICE_ACC_MXC6202XH)
#define MXC6202X_OFFSET_X		512
#define MXC6202X_OFFSET_Y		512
#define MXC6202X_OFFSET_Z		512
#define MXC6202X_SENSITIVITY_X		128
#define MXC6202X_SENSITIVITY_Y		128
#define MXC6202X_SENSITIVITY_Z		128
#elif (defined DEVICE_ACC_MXC6202XM)
#define MXC6202X_OFFSET_X		2048
#define MXC6202X_OFFSET_Y		2048
#define MXC6202X_OFFSET_Z		2048
#define MXC6202X_SENSITIVITY_X		512
#define MXC6202X_SENSITIVITY_Y		512
#define MXC6202X_SENSITIVITY_Z		512
#elif (defined DEVICE_ACC_MXC6202XN)
#define MXC6202X_OFFSET_X		512
#define MXC6202X_OFFSET_Y		512
#define MXC6202X_OFFSET_Z		512
#define MXC6202X_SENSITIVITY_X		128
#define MXC6202X_SENSITIVITY_Y		128
#define MXC6202X_SENSITIVITY_Z		128
#endif

/**
 * NOTE:
 * You are required to get the correct install direction according 
 * the sensor placement on target board 
 */
#define MXC6202X_INSTALL_DIR		5

#if (defined DEVICE_ACC_MXC6202X)

#include <stdio.h>
#include <fcntl.h>
#include <sys/ioctl.h>

#define LOG_TAG "SensorACC"
#include <utils/Log.h>

#include <sensors_acc_mxc6202x.h>

/* Use 'm' as magic number */
#define MXC6202X_IOM			'm'

/* IOCTLs for MXC6202X device */
#define MXC6202X_IOC_PWRON		_IO (MXC6202X_IOM, 0x00)
#define MXC6202X_IOC_PWRDN		_IO (MXC6202X_IOM, 0x01)
#define MXC6202X_IOC_ST			_IO (MXC6202X_IOM, 0x02)
#define MXC6202X_IOC_BGTST		_IO (MXC6202X_IOM, 0x03)
#define MXC6202X_IOC_TOEN		_IO (MXC6202X_IOM, 0x04)
#define MXC6202X_IOC_READXYZ		_IOR(MXC6202X_IOM, 0x05, int[3])
#define MXC6202X_IOC_READTEMP		_IOR(MXC6202X_IOM, 0x06, int)

int acc_mxc6202x_init(void)
{
	return 0;
}

int acc_mxc6202x_open(void)
{
	int fd;
	fd = open("/dev/mxc6202x", O_RDWR);
	if (fd < 0) {
		return -1;
	}

	return fd;
}

int acc_mxc6202x_close(int fd)
{
	close(fd);

	return 0;
}

int acc_mxc6202x_read_data(int fd, int *data)
{
	int res = ioctl(fd, MXC6202X_IOC_READXYZ, data);
	data[2] = MXC6202X_OFFSET_Z;
	return res;
}

int acc_mxc6202x_get_offset(int fd, int *offset_xyz)
{
	offset_xyz[0] = MXC6202X_OFFSET_X;
	offset_xyz[1] = MXC6202X_OFFSET_Y;
	offset_xyz[2] = MXC6202X_OFFSET_Z;

	return 0;
}

int acc_mxc6202x_set_new_offset(int fd, int *offset_xyz)
{
	return 0;
}

int acc_mxc6202x_get_sensitivity(int fd, int *sensit_xyz)
{
	sensit_xyz[0] = MXC6202X_SENSITIVITY_X;
	sensit_xyz[1] = MXC6202X_SENSITIVITY_Y;
	sensit_xyz[2] = MXC6202X_SENSITIVITY_Z;

	return 0;
}

int acc_mxc6202x_get_install_dir(void)
{
	return MXC6202X_INSTALL_DIR;
}

#endif /* DEVICE_ACC_MXC6202X */

