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
 * This file define sensor placement on target board and 
 * utilities related coordiante system.
 */

#ifndef __SENSORS_COORDINATE_H__
#define __SENSORS_COORDINATE_H__

/*
 * Definition of sensor placement on target board
 * 
 *               up
 *       +---------------+
 *       |+-------------+|
 *       ||             ||
 * left  ||             || right
 *       ||             ||
 *       ||   screen    ||
 *       ||             ||
 *       ||             ||
 *       |+-------------+|
 *       |---------------|
 *       |[ ] [ ] [ ] [ ]|
 *       | 1   2   3   * |
 *       | 4   5   6   0 |
 *       | 7   8   9   # |
 *       +---------------+
 *              down
 *
 *                                left    right   up      down
 * OBVERSE_X_AXIS_FORWARD          Y+      Y-      X+      X-     ->     0
 * OBVERSE_X_AXIS_RIGHTWARD        X-      X+      Y+      Y-     ->     1
 * OBVERSE_X_AXIS_BACKWARD         Y-      Y+      X-      X+     ->     2
 * OBVERSE_X_AXIS_LEFTWARD         X+      X-      Y-      Y+     ->     3
 * REVERSE_X_AXIS_FORWARD          Y-      Y+      X+      X-     ->     4
 * REVERSE_X_AXIS_RIGHTWARD        X-      X+      Y-      Y+     ->     5
 * REVERSE_X_AXIS_BACKWARD         Y+      Y-      X-      X+     ->     6
 * REVERSE_X_AXIS_LEFTWARD         X+      X-      Y+      Y-     ->     7
 */

/**
 * @brief
 * Sensor placement on target board
 */
enum sensors_placement_t {
	OBVERSE_X_AXIS_FORWARD,		/*!< Sensor on the same side of screen, x-axis point forward */
	OBVERSE_X_AXIS_RIGHTWARD,	/*!< Sensor on the same side of screen, x-axis point rightward */
	OBVERSE_X_AXIS_BACKWARD,	/*!< Sensor on the same side of screen, x-axis point backward */
	OBVERSE_X_AXIS_LEFTWARD,	/*!< Sensor on the same side of screen, x-axis point leftward */
	REVERSE_X_AXIS_FORWARD,		/*!< Sensor on the reverse side of screen, x-axis point forward */
	REVERSE_X_AXIS_RIGHTWARD,	/*!< Sensor on the reverse side of screen, x-axis point rightward */
	REVERSE_X_AXIS_BACKWARD,	/*!< Sensor on the reverse side of screen, x-axis point backward */
	REVERSE_X_AXIS_LEFTWARD		/*!< Sensor on the reverse side of screen, x-axis point leftward */
};

/**
 * @brief Convert sensor offset vector to target coordinate.
 * @param vec_io is the offset vector.
 * @param dir is the sensor placement on target board.
 */
void coordinate_offset_convert(int *vec_io, int dir);
/**
 * @brief Convert sensor sensitivity vector to target coordinate.
 * @param vec_io is the sensitivity vector.
 * @param dir is the sensor placement on target board.
 */
void coordinate_sensitivity_convert(int *vec_io, int dir);

/**
 * @brief Convert sensor raw data vector to android coordinate.
 * @param vec_io is the raw data vector.
 * @param offset is the offset vector.
 * @param dir is the sensor placement on target board.
 */
void coordinate_raw_to_android(int *vec_io, const int *offset, int dir);
/**
 * @brief Convert sensor raw data vector to IDS coordinate.
 * @param out is the raw data vector in IDS coordinate.
 * @param in is the raw data vector from sensor device.
 * @param offset is the offset vector.
 * @param dir is the sensor placement on target board.
 */
void coordinate_raw_to_ids(int *out, const int *in, const int *offset, int dir);

/**
 * @brief Convert sensor real data(in physical unit) vector to android coordinate.
 * @param vec_out is the real data vector in android coordinate.
 * @param vec_in is the real data vector from sensor.
 * @param dir is the sensor placement on target board.
 */
void coordinate_real_to_android(float *vec_out, const float *vec_in, int dir);
/**
 * @brief Convert sensor real data(in physical unit) vector to IDS coordinate.
 * @param vec_out is the real data vector in IDS coordinate.
 * @param vec_in is the real data vector from sensor.
 * @param dir is the sensor placement on target board.
 */
void coordinate_real_to_ids(float *vec_out, const float *vec_in, int dir);

#endif /* __SENSORS_COORDINATE_H__ */

