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
 * This file implement the interface of eCompass algorithm utilities.
 */

#if ((defined COMPASS_ALGO_H5) ||	\
     (defined COMPASS_ALGO_H6))

#include <stdio.h>
#include <sys/time.h>
#include <time.h>

#include <sensors_data_struct.h>
#include <sensors_algo_ids_util.h>

int ids_get_milliseconds()
{
	struct timeval tv_cur;

	gettimeofday(&tv_cur, NULL);
	return (tv_cur.tv_sec * 1000 + tv_cur.tv_usec / 1000) % 65536;
}

/*
 * Assume offset is (1 << n), here n <= 16 
 * The return value is (16 - n)
 */
int ids_get_shitcount(int offset)
{
	unsigned int tmp;
	int count = 0;
	int i;

	tmp = (unsigned int)offset;
	if (!tmp) {
		return 0;
	}
	for (i = 1; ; i++) {
		if ((tmp << i) > 32768) {
			break;
		}
		count++;
	}

	return count;
}

float ids_degree_to_angle(int in)
{
	float tmp;

	tmp = (float)in;
	tmp *= (float)360;
	tmp /= (float)65536;

	return tmp;
}

float ids_degree_to_angle_positive(int in)
{
	int tmp;

	tmp = (unsigned short)in;

	return ((float)tmp * 360 / 65536);
}

int ids_degree_tilt_from_raw(int value, int offset, int sensit)
{
	if (value >= offset + sensit) {
		return 32767;
	}
	if (value <= offset - sensit) {
		return -32768;
	}

	return (value - offset) * 32768 / sensit;
}

int ids_degree_tilt_from_real(float value)
{
	if (value >= 1.0) {
		return 32767;
	}
	if (value <= -1.0) {
		return -32768;
	}

	return (int)(value * 32768);
}

int ids_degree_mag_from_real(float value)
{
	const int OFFSET = 4096;
	const int SENSITIVITY = 512;
	const int SHIFT = 3;
	uint16 raw;

	raw = (uint16)(OFFSET + value * SENSITIVITY);

	return (raw << SHIFT);
}

void ids_degree_real_to_algo(struct SensorData_Algo *dout, const struct SensorData_Real *din)
{
	dout->gx = (int16)ids_degree_tilt_from_real(din->acc[0]);
	dout->gy = (int16)ids_degree_tilt_from_real(din->acc[1]);
	dout->gz = (int16)ids_degree_tilt_from_real(din->acc[2]);

	dout->hx = (uint16)ids_degree_mag_from_real(din->mag[0]);
	dout->hy = (uint16)ids_degree_mag_from_real(din->mag[1]);
	dout->hz = (uint16)ids_degree_mag_from_real(din->mag[2]);
}

#endif /* COMPASS_ALGO_H5 || COMPASS_ALGO_H6 */

