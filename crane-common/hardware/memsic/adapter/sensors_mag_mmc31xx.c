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
 * This file implement magnetic sensor adapter for MMC31xx.
 */

#if ((defined DEVICE_MAG_MMC312X) ||	\
     (defined DEVICE_MAG_MMC314X))
#define DEVICE_MAG_MMC31XX
#endif

#if (defined DEVICE_MAG_MMC312X)
#define MMC31XX_OFFSET_X		2048
#define MMC31XX_OFFSET_Y		2048
#define MMC31XX_OFFSET_Z		2048
#define MMC31XX_SENSITIVITY_X		512
#define MMC31XX_SENSITIVITY_Y		512
#define MMC31XX_SENSITIVITY_Z		512
#elif (defined DEVICE_MAG_MMC314X)
#define MMC31XX_OFFSET_X		4096
#define MMC31XX_OFFSET_Y		4096
#define MMC31XX_OFFSET_Z		4096
#define MMC31XX_SENSITIVITY_X		512
#define MMC31XX_SENSITIVITY_Y		512
#define MMC31XX_SENSITIVITY_Z		512
#endif

/**
 * NOTE:
 * You are required to get the correct install direction according 
 * the sensor placement on target board 
 */
#define MMC31XX_INSTALL_DIR		7

#if (defined DEVICE_MAG_MMC31XX)

#include <stdio.h>
#include <fcntl.h>
#include <sys/ioctl.h>

#define LOG_TAG "SensorMAG"
#include <utils/Log.h>

#include <sensors_mag_mmc31xx.h>

/* Use 'm' as magic number */
#define MMC31XX_IOM			'm'

/* IOCTLs for MMC31XX device */
#define MMC31XX_IOC_TM			_IO (MMC31XX_IOM, 0x00)
#define MMC31XX_IOC_SET			_IO (MMC31XX_IOM, 0x01)
#define MMC31XX_IOC_RESET		_IO (MMC31XX_IOM, 0x02)
#define MMC31XX_IOC_READ		_IOR(MMC31XX_IOM, 0x03, int[3])
#define MMC31XX_IOC_READXYZ		_IOR(MMC31XX_IOM, 0x04, int[3])

int mag_mmc31xx_init(void)
{
	return 0;
}

int mag_mmc31xx_open(void)
{
	int fd;
#if 0
	fd = open("/dev/mmc31xx", O_RDWR);
	if (fd < 0) {
		return -1;
	}
#else
	fd = 5;
#endif
	return fd;
}

int mag_mmc31xx_close(int fd)
{
#if 0
	close(fd);
#endif

	return 0;
}

int mag_mmc31xx_read_data(int fd, int *data)
{
#if 0
	return ioctl(fd, MMC31XX_IOC_READXYZ, data);
#else
	data[0] = data[1] = data[2] = 4096; 
	return 1;
#endif
}

int mag_mmc31xx_get_offset(int fd, int *offset_xyz)
{
	offset_xyz[0] = MMC31XX_OFFSET_X;
	offset_xyz[1] = MMC31XX_OFFSET_Y;
	offset_xyz[2] = MMC31XX_OFFSET_Z;

	return 0;
}

int mag_mmc31xx_get_sensitivity(int fd, int *sensit_xyz)
{
	sensit_xyz[0] = MMC31XX_SENSITIVITY_X;
	sensit_xyz[1] = MMC31XX_SENSITIVITY_Y;
	sensit_xyz[2] = MMC31XX_SENSITIVITY_Z;

	return 0;
}

int mag_mmc31xx_get_install_dir(void)
{
	return MMC31XX_INSTALL_DIR;
}

#endif /* DEVICE_MAG_MMC31XX */

