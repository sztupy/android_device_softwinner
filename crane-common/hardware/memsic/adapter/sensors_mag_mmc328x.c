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
 *          Dale Hou<byhou@memsic.cn>
 * @brief
 * This file implement magnetic sensor adapter for MMC328x.
 */


#define MMC328X_OFFSET_X		4096
#define MMC328X_OFFSET_Y		4096
#define MMC328X_OFFSET_Z		4096
#define MMC328X_SENSITIVITY_X		512
#define MMC328X_SENSITIVITY_Y		512
#define MMC328X_SENSITIVITY_Z		512

/**
 * NOTE:
 * You are required to get the correct install direction according 
 * the sensor placement on target board 
 */
#define MMC328X_INSTALL_DIR		4

#if (defined DEVICE_MAG_MMC328X)

#include <stdio.h>
#include <fcntl.h>
#include <sys/ioctl.h>

#define LOG_TAG "SensorMAG"
#include <utils/Log.h>

#include <sensors_mag_mmc328x.h>

/* Use 'm' as magic number */
#define MMC328X_IOM			'm'

/* IOCTLs for MMC328X device */
#define MMC328X_IOC_TM			_IO (MMC328X_IOM, 0x00)
#define MMC328X_IOC_RM			_IO (MMC328X_IOM, 0x01)
#define MMC328X_IOC_READ		_IOR(MMC328X_IOM, 0x02, int[3])
#define MMC328X_IOC_READXYZ		_IOR(MMC328X_IOM, 0x03, int[3])

int mag_mmc328x_init(void)
{
	return 0;
}

int mag_mmc328x_open(void)
{
	int fd;

	fd = open("/dev/mmc328x", O_RDWR);
	if (fd < 0) {
		return -1;
	}

	return fd;
}

int mag_mmc328x_close(int fd)
{
	close(fd);

	return 0;
}

int mag_mmc328x_read_data(int fd, int *data)
{
	return ioctl(fd, MMC328X_IOC_READXYZ, data);
}

int mag_mmc328x_get_offset(int fd, int *offset_xyz)
{
	offset_xyz[0] = MMC328X_OFFSET_X;
	offset_xyz[1] = MMC328X_OFFSET_Y;
	offset_xyz[2] = MMC328X_OFFSET_Z;

	return 0;
}

int mag_mmc328x_get_sensitivity(int fd, int *sensit_xyz)
{
	sensit_xyz[0] = MMC328X_SENSITIVITY_X;
	sensit_xyz[1] = MMC328X_SENSITIVITY_Y;
	sensit_xyz[2] = MMC328X_SENSITIVITY_Z;

	return 0;
}

int mag_mmc328x_get_install_dir(void)
{
	return MMC328X_INSTALL_DIR;
}

#endif /* DEVICE_MAG_MMC328X */

