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
 * This file define the interface of eCompass algorithm adapter for 3+3.
 */

#ifndef __SENSORS_ALGO_IDS_H6_H__
#define __SENSORS_ALGO_IDS_H6_H__

int ids_h6_init(void);

int ids_h6_open(void);
int ids_h6_close(void);
int ids_h6_restart(void);
int ids_h6_get_state(void);
void ids_h6_clear_state(void);

int ids_h6_nvm_load(void);
int ids_h6_nvm_save(void);
int ids_h6_nvm_restore(void);

void ids_h6_data_in(struct SensorData_Algo *d);
int ids_h6_calc_orientation(struct SensorData_Orientation *o, struct SensorData_Algo *d);
int ids_h6_calibration(struct SensorData_Algo *d);

int ids_h6_calc_magcentre(const float mag[3], float centre[3]);
int ids_h6_calc_magcal_data(struct SensorData_Algo *d, float cald[3]);

#endif /* __SENSORS_ALGO_IDS_H6_H__ */

