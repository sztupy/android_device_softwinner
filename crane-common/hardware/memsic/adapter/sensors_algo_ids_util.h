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
 * This file define the interface of eCompass algorithm utilities.
 */

#ifndef __SENSORS_ALGO_IDS_UTIL_H__
#define __SENSORS_ALGO_IDS_UTIL_H__

#include <sensors_data_struct.h>

int ids_get_milliseconds(void);
int ids_get_shitcount(int offset);

float ids_degree_to_angle(int in);
float ids_degree_to_angle_positive(int in);
int ids_degree_tilt_from_raw(int value, int offset, int sensit);
int ids_degree_tilt_from_real(float value);
void ids_degree_real_to_algo(struct SensorData_Algo *dout, const struct SensorData_Real *din);

#endif /* __SENSORS_ALGO_IDS_UTIL_H__ */

