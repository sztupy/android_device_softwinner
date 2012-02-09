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
 * This file implement eCompass algorithm adapter APIs.
 */

#include <sensors_algo_adapter.h>

#if (defined COMPASS_ALGO_H5)
#include <sensors_algo_ids_h5.h>
#elif (defined COMPASS_ALGO_H6)
#include <sensors_algo_ids_h6.h>
#else
#error "Algorithm not specify!"
#endif

static struct algo_t algo = {
#if (defined COMPASS_ALGO_H5)
	init		: ids_h5_init,

	open		: ids_h5_open,
	close		: ids_h5_close,
	restart		: ids_h5_restart,
	get_state	: ids_h5_get_state,
	clear_state	: ids_h5_clear_state,

	nvm_load	: ids_h5_nvm_load,
	nvm_store	: ids_h5_nvm_save,
	nvm_restore	: ids_h5_nvm_restore,

	data_in		: ids_h5_data_in,
	calc_orientation: ids_h5_calc_orientation,
	calc_magcentre	: ids_h5_calc_magcentre,
	calc_magcal_data: ids_h5_calc_magcal_data,
	calibrate	: ids_h5_calibration
#elif (defined COMPASS_ALGO_H6)
	init		: ids_h6_init,

	open		: ids_h6_open,
	close		: ids_h6_close,
	restart		: ids_h6_restart,
	get_state	: ids_h6_get_state,
	clear_state	: ids_h6_clear_state,

	nvm_load	: ids_h6_nvm_load,
	nvm_store	: ids_h6_nvm_save,
	nvm_restore	: ids_h6_nvm_restore,

	data_in		: ids_h6_data_in,
	calc_orientation: ids_h6_calc_orientation,
	calc_magcentre	: ids_h6_calc_magcentre,
	calc_magcal_data: ids_h6_calc_magcal_data,
	calibrate	: ids_h6_calibration
#else
#error "Algorithm not specify!"
#endif
};

struct algo_t *sensors_get_algorithm(void)
{
	return &algo;
}

