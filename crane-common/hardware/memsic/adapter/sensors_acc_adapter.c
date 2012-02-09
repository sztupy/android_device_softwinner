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
 * This file implement the interface of acceleration adapter.
 */

#include <sensors_acc_adapter.h>

#if ((defined DEVICE_ACC_MXC6202XG) ||	\
     (defined DEVICE_ACC_MXC6202XH) ||	\
     (defined DEVICE_ACC_MXC6202XM) ||	\
     (defined DEVICE_ACC_MXC6202XN))
#define DEVICE_ACC_MXC6202X
#endif

#if (defined DEVICE_ACC_MXC6202X)
#include <sensors_acc_mxc6202x.h>
#elif (defined DEVICE_ACC_MXC622X)
#include <sensors_acc_mxc622x.h>
#elif (defined DEVICE_ACC_BMA220)
#include <sensors_acc_bma220.h>
#elif (defined DEVICE_ACC_BMA023)
#include <sensors_acc_bma023.h>
#elif (defined DEVICE_ACC_LIS33DE)
#include <sensors_acc_lis33de.h>
#else
#error "Acc sensor device not specify!"
#endif

static struct device_acc_t dev_acc = {
#if (defined DEVICE_ACC_MXC6202X)
	init		: acc_mxc6202x_init,

	open		: acc_mxc6202x_open,
	close		: acc_mxc6202x_close,

	read_data	: acc_mxc6202x_read_data,
	get_offset	: acc_mxc6202x_get_offset,
	set_new_offset	: acc_mxc6202x_set_new_offset,
	get_sensitivity	: acc_mxc6202x_get_sensitivity,
	get_install_dir	: acc_mxc6202x_get_install_dir
#elif (defined DEVICE_ACC_LIS33DE)
	init		: acc_lis33de_init,
	open		: acc_lis33de_open,
	close		: acc_lis33de_close,
	read_data	: acc_lis33de_read_data,
	get_offset	: acc_lis33de_get_offset,
	get_sensitivity	: acc_lis33de_get_sensitivity,
	get_install_dir	: acc_lis33de_get_install_dir
#elif (defined DEVICE_ACC_MXC622X)
	init		: acc_mxc622x_init,

	open		: acc_mxc622x_open,
	close		: acc_mxc622x_close,

	read_data	: acc_mxc622x_read_data,
	get_offset	: acc_mxc622x_get_offset,
	set_new_offset	: acc_mxc622x_set_new_offset,
	get_sensitivity	: acc_mxc622x_get_sensitivity,
	get_install_dir	: acc_mxc622x_get_install_dir
#elif (defined DEVICE_ACC_BMA220)
	init		: acc_bma220_init,

	open		: acc_bma220_open,
	close		: acc_bma220_close,

	read_data	: acc_bma220_read_data,
	get_offset	: acc_bma220_get_offset,
	set_new_offset	: acc_bma220_set_new_offset,
	get_sensitivity	: acc_bma220_get_sensitivity,
	get_install_dir	: acc_bma220_get_install_dir
#elif (defined DEVICE_ACC_BMA023)
	init		: acc_bma023_init,

	open		: acc_bma023_open,
	close		: acc_bma023_close,

	read_data	: acc_bma023_read_data,
	get_offset	: acc_bma023_get_offset,
	set_new_offset	: acc_bma023_set_new_offset,
	get_sensitivity	: acc_bma023_get_sensitivity,
	get_install_dir	: acc_bma023_get_install_dir
#else
	#error "Acc sensor device not specify!"
#endif
};

struct device_acc_t *sensors_get_acc_device(void)
{
	return &dev_acc;
}

