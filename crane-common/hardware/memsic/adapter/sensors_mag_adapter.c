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
 * This file implement magnetic sensor adapter APIs.
 */

#include <sensors_mag_adapter.h>

#if ((defined DEVICE_MAG_MMC312X) ||	\
     (defined DEVICE_MAG_MMC314X))
#define DEVICE_MAG_MMC31XX
#endif

#if (defined DEVICE_MAG_MMC31XX)
#include <sensors_mag_mmc31xx.h>
#elif (defined DEVICE_MAG_MMC328X)
#include <sensors_mag_mmc328x.h>
#else
#error "Mag sensor device not specify!"
#endif

static struct device_mag_t dev_mag = {

#if (defined DEVICE_MAG_MMC31XX)
	init		: mag_mmc31xx_init,

	open		: mag_mmc31xx_open,
	close		: mag_mmc31xx_close,

	read_data	: mag_mmc31xx_read_data,
	get_offset	: mag_mmc31xx_get_offset,
	get_sensitivity	: mag_mmc31xx_get_sensitivity,
	get_install_dir	: mag_mmc31xx_get_install_dir
#elif (defined DEVICE_MAG_MMC328X)
	init		: mag_mmc328x_init,

	open		: mag_mmc328x_open,
	close		: mag_mmc328x_close,

	read_data	: mag_mmc328x_read_data,
	get_offset	: mag_mmc328x_get_offset,
	get_sensitivity	: mag_mmc328x_get_sensitivity,
	get_install_dir	: mag_mmc328x_get_install_dir
#else
	#error "Mag sensor device not specify!"
#endif
};

struct device_mag_t *sensors_get_mag_device(void)
{
	return &dev_mag;
}

