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
 * This file implement utilities related coordiante system.
 */

#include <stdio.h>
#include <sensors_coordinate.h>

void coordinate_offset_convert(int *vec_io, int dir)
{
	int tmp;

	if (!vec_io) {
		return;
	}

	switch (dir) {
	case 0:
	case 2:
	case 4:
	case 6:
		//x'=y y'=x z'=z
		tmp = vec_io[0];
		vec_io[0] = vec_io[1];
		vec_io[1] = tmp;
		break;
	case 1:
	case 3:
	case 5:
	case 7:
		//x'=x y'=y z'=z
		break;
	default:
		break;
	}
}

void coordinate_sensitivity_convert(int *vec_io, int dir)
{
	int tmp;

	if (!vec_io) {
		return;
	}

	switch (dir) {
	case 0:
	case 2:
	case 4:
	case 6:
		//x'=y y'=x z'=z
		tmp = vec_io[0];
		vec_io[0] = vec_io[1];
		vec_io[1] = tmp;
		break;
	case 1:
	case 3:
	case 5:
	case 7:
		//x'=x y'=y z'=z
		break;
	default:
		break;
	}
}

/*
 * convert raw data to coordinate system according to 
 * android HAL's definition in sensors.h
 */
void coordinate_raw_to_android(int *vec_io, const int *offset, int dir)
{
	int tmp;

	if ((!vec_io) || (!offset)) {
		return;
	}

	switch (dir) {
	case 0:
		//x'=-y y'=x z'=z
		tmp = vec_io[0];
		vec_io[0] = (offset[1] << 1) - vec_io[1];
		vec_io[1] = tmp;
		break;
	case 1:
		//x'=x y'=y z'=z
		break;
	case 2:
		//x'=y y'=-x z'=z
		tmp = vec_io[0];
		vec_io[0] = vec_io[1];
		vec_io[1] = (offset[0] << 1) - tmp;
		break;
	case 3:
		//x'=-x y'=-y z'=z
		vec_io[0] = (offset[0] << 1) - vec_io[0];
		vec_io[1] = (offset[1] << 1) - vec_io[1];
		break;
	case 4:
		//x'=y y'=x z'=-z
		tmp = vec_io[0];
		vec_io[0] = vec_io[1];
		vec_io[1] = tmp;
		vec_io[2] = (offset[2] << 1)- vec_io[2];
		break;
	case 5:
		//x'=x y'=-y z'=-z
		vec_io[0] = vec_io[0];
		vec_io[1] = (offset[1] << 1) - vec_io[1];
		vec_io[2] = (offset[2] << 1) - vec_io[2];
		break;
	case 6:
		//x'=-y y'=-x z'=-z
		tmp = vec_io[0];
		vec_io[0] = (offset[1] << 1) - vec_io[1];
		vec_io[1] = (offset[0] << 1) - tmp;
		vec_io[2] = (offset[2] << 1) - vec_io[2];
		break;
	case 7:
		//x'=-x y'=y z'=-z
		vec_io[0] = (offset[0] << 1) - vec_io[0];
		vec_io[1] = vec_io[1];
		vec_io[2] = (offset[2] << 1) - vec_io[2];
		break;
	default:
		break;
	}
}

/*
 * convert coordinate system to meet compassLib algo's need
 */
void coordinate_raw_to_ids(int *out, const int *in, const int *offset, int dir)
{
	if ((!out) || (!in) || (!offset)) {
		return;
	}

	switch (dir) {
	case 0:
		//x'=x y'=-y z'=-z
		out[0] = in[0];
		out[1] = (offset[1] << 1) - in[1];
		out[2] = (offset[2] << 1) - in[2];
		break;
	case 1:
		//x'=y y'=x z'=-z
		out[0] = in[1];
		out[1] = in[0];
		out[2] = (offset[2] << 1) - in[2];
		break;
	case 2:
		//x'=-x y'=y z'=-z
		out[0] = (offset[0] << 1) - in[0];
		out[1] = in[1];
		out[2] = (offset[2] << 1) - in[2];
		break;
	case 3:
		//x'=-y y'=-x z'=-z
		out[0] = (offset[1] << 1) - in[1];
		out[1] = (offset[0] << 1) - in[0];
		out[2] = (offset[2] << 1) - in[2];
		break;
	case 4:
		//x'=x y'=y z'=z
		out[0] = in[0];
		out[1] = in[1];
		out[2] = in[2];
		break;
	case 5:
		//x'=-y y'=x z'=z
		out[0] = (offset[1] << 1) - in[1];
		out[1] = in[0];
		out[2] = in[2];
		break;
	case 6:
		//x'=-x y'=-y z'=z
		out[0] = (offset[0] << 1) - in[0];
		out[1] = (offset[1] << 1) - in[1];
		out[2] = in[2];
		break;
	case 7:
		//x'=y y'=-x z'=z
		out[0] = in[1];
		out[1] = (offset[0] << 1) - in[0];
		out[2] = in[2];
		break;
	default:
		//x'=x y'=y z'=z
		out[0] = in[0];
		out[1] = in[1];
		out[2] = in[2];
		break;
	}
}

void coordinate_real_to_android(float *out, const float *in, int dir)
{
	if ((!out) || (!in)) {
		return;
	}

	switch (dir) {
	case 0:
		//x'=-y y'=x z'=z
		out[0] = -in[1];
		out[1] = in[0];
		out[2] = in[2];
		break;
	case 1:
		//x'=x y'=y z'=z
		out[0] = in[0];
		out[1] = in[1];
		out[2] = in[2];
		break;
	case 2:
		//x'=y y'=-x z'=z
		out[0] = in[1];
		out[1] = -in[0];
		out[2] = in[2];
		break;
	case 3:
		//x'=-x y'=-y z'=z
		out[0] = -in[0];
		out[1] = -in[1];
		out[2] = in[2];
		break;
	case 4:
		//x'=y y'=x z'=-z
		out[0] = in[1];
		out[1] = in[0];
		out[2] = -in[2];
		break;
	case 5:
		//x'=x y'=-y z'=-z
		out[0] = in[0];
		out[1] = -in[1];
		out[2] = -in[2];
		break;
	case 6:
		//x'=-y y'=-x z'=-z
		out[0] = -in[1];
		out[1] = -in[0];
		out[2] = -in[2];
		break;
	case 7:
		//x'=-x y'=y z'=-z
		out[0] = -in[0];
		out[1] = in[1];
		out[2] = -in[2];
		break;
	default:
		//x'=x y'=y z'=z
		out[0] = in[0];
		out[1] = in[1];
		out[2] = in[2];
		break;
	}
}

void coordinate_real_to_ids(float *out, const float *in, int dir)
{
	if ((!out) || (!in)) {
		return;
	}

	switch (dir) {
	case 0:
		//x'=x y'=-y z'=-z
		out[0] = in[0];
		out[1] = -in[1];
		out[2] = -in[2];
		break;
	case 1:
		//x'=y y'=x z'=-z
		out[0] = in[1];
		out[1] = in[0];
		out[2] = -in[2];
		break;
	case 2:
		//x'=-x y'=y z'=-z
		out[0] = -in[0];
		out[1] = in[1];
		out[2] = -in[2];
		break;
	case 3:
		//x'=-y y'=-x z'=-z
		out[0] = -in[1];
		out[1] = -in[0];
		out[2] = -in[2];
		break;
	case 4:
		//x'=x y'=y z'=z
		out[0] = in[0];
		out[1] = in[1];
		out[2] = in[2];
		break;
	case 5:
		//x'=-y y'=x z'=z
		out[0] = -in[1];
		out[1] = in[0];
		out[2] = in[2];
		break;
	case 6:
		//x'=-x y'=-y z'=z
		out[0] = -in[0];
		out[1] = -in[1];
		out[2] = in[2];
		break;
	case 7:
		//x'=y y'=-x z'=z
		out[0] = in[1];
		out[1] = -in[0];
		out[2] = in[2];
		break;
	default:
		//x'=x y'=y z'=z
		out[0] = in[0];
		out[1] = in[1];
		out[2] = in[2];
		break;
	}
}

