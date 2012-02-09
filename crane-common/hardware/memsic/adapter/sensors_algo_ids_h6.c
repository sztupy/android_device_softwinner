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
 * This file implement the interface of eCompass algorithm adapter for 3+3.
 */

#if (defined COMPASS_ALGO_H6)

#define LOG_TAG "SensorAlgo"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <assert.h>
#include <time.h>
#include <math.h>
#include <sys/mman.h>

#include <CompassLib_H6.h>
#include <sensors_algo_adapter.h>
#include <sensors_algo_ids_util.h>
#include <sensors_coordinate.h>
#include <sensors_data_struct.h>

#ifdef MODULE_DEBUG
#include <utils/Log.h>
#endif

#define SENSOR_STATUS_UNRELIABLE	0
#define SENSOR_STATUS_ACCURACY_LOW	1
#define SENSOR_STATUS_ACCURACY_MEDIUM	2
#define SENSOR_STATUS_ACCURACY_HIGH	3

#define SENSORS_ALGO_DEBUG	1

#if (!defined MODULE_DEBUG)
#if SENSORS_ALGO_DEBUG
#define LOGD(x...)		printf(x)
#define LOGE(x...)		printf(x)
#else
#define LOGD(x...)
#define LOGE(x...)
#endif
#endif

#define NVM_PATH		"/data/misc/sensors/ecs_nvm"
#define NVM_SIZE		(26 + 2)
#define NVM_STATE_BYTE		26
#define MAPPED_MEM_SIZE		256

#define FILTER			0

#define BUFFER_DUMP		0

#define AVG_BUFF_SIZE		16
#define MAG_MAX			4.5	// > 4.0 (MMC314x MaxRange)
#define MAG_NOI_THRESHOLD	20.0

struct magnetic_cali_buff {
	float vx[AVG_BUFF_SIZE];
	float vy[AVG_BUFF_SIZE];
	float vz[AVG_BUFF_SIZE];
};

static struct magnetic_cali_buff mcb_max;	// ascending buffer
static struct magnetic_cali_buff mcb_min;	// descending buffer

static uint8 *NVM = NULL;

static uint8 NVM_RAM[NVM_SIZE] =
{
	0xff, 0x7f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0xff, 0x7f, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,0x00,
	0x00, 0x4d,

	// two more byte for algo state
	0x00, 0x00
};

static uint8 NVM_Restore[NVM_SIZE] =
{
	0xff, 0x7f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0xff, 0x7f, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,0x00,
	0x00, 0x4d,

	// two more byte for algo state
	0x00, 0x00
};

/* initial parameter must be global virable */
static COMPASSLIB_H6_INIT_STRUCT init_parm;

int ids_h6_nvm_restore()
{
	memcpy(NVM, NVM_Restore, NVM_SIZE);
	NVM[NVM_STATE_BYTE] = 1;

	if (NVM != NVM_RAM) {
		msync(NVM, NVM_SIZE, MS_SYNC);
	}
	return 0;
}

int ids_h6_nvm_load()
{
	int fd;
	int n;
	uint8 buf[NVM_SIZE];

	fd = open(NVM_PATH, O_RDWR | O_CREAT, 0666);
	if (fd == -1) {
		LOGE("%s: fail to open %s\n", __FUNCTION__, NVM_PATH);
		NVM = NVM_RAM;
		return -1;
	}

	lseek(fd, 0, SEEK_SET);
	n = read(fd, buf, NVM_SIZE);
	if (n != NVM_SIZE) {	// empty or broken parm file
		// restore parm file to default
		lseek(fd, 0, SEEK_SET);
		n = write(fd, NVM_Restore, NVM_SIZE);
	}
	NVM = mmap(0, MAPPED_MEM_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
	if (NVM == MAP_FAILED) {
		// set NVM to RAM if failure
		NVM = NVM_RAM;
	}
	close(fd);

	return 0;
}

int ids_h6_nvm_save()
{
	if (NVM != NVM_RAM) {
		msync(NVM, NVM_SIZE, MS_SYNC);
	}
	return 0;
}

uint16 ids_h6_nvm_read(uint16 offset)
{
	if (offset < CompassLib_H6_GetNVMBlockSize()) {
		return (uint8)NVM[offset];
	}

	return 0xFFFF;
}

uint8 ids_h6_nvm_write(uint16 offset, uint8 data)
{
	if (offset < CompassLib_H6_GetNVMBlockSize()) {
		NVM[offset] = data;
		return 1;
	}

	return 0;
}

int ids_h6_init(void)
{
	COMPASSLIB_H6_BUILD_DATE builddate;
	COMPASSLIB_H6_INIT_RESULT initRes;
	int i;

	for (i = 0; i < AVG_BUFF_SIZE; i++) {
		mcb_max.vx[i] = -MAG_MAX;
		mcb_max.vy[i] = -MAG_MAX;
		mcb_max.vz[i] = -MAG_MAX;
		mcb_min.vx[i] = MAG_MAX;
		mcb_min.vy[i] = MAG_MAX;
		mcb_min.vz[i] = MAG_MAX;
	}
	// Bring non-volatile memory online first of all
	ids_h6_nvm_load();

#if SENSORS_ALGO_DEBUG
	builddate = CompassLib_H6_GetBuildDate();
	LOGD("Algorithm Library Info:\n");
	LOGD("Build date: %02x-%02x-%02x, %02x:%02x:%02x\n",
		builddate.Year, builddate.Month, builddate.Day,
		builddate.Hour, builddate.Minute, builddate.Second);
	LOGD("NVM block size: %d\n", CompassLib_H6_GetNVMBlockSize());
	LOGD("Version: %04x\n", CompassLib_H6_GetVersion());
	LOGD("Auto-Calibration: %d\n", CompassLib_H6_IsAutoCalibrationEnabled());
#endif

	init_parm.Size = sizeof(COMPASSLIB_H6_INIT_STRUCT);
	init_parm.VersionHi = COMPASSLIB_H6_VERSION_HI;
	init_parm.VersionLo = COMPASSLIB_H6_VERSION_LO;
	init_parm.NVM_Read = ids_h6_nvm_read;
	init_parm.NVM_Write = ids_h6_nvm_write;
	initRes = CompassLib_H6_Init(&init_parm);

	LOGD("CompassLib init result: %d\n", initRes);

	return initRes;
}

int ids_h6_open(void)
{
	if (!NVM) {
		ids_h6_nvm_load();
	}
	return 0;
}

int ids_h6_close(void)
{
	ids_h6_nvm_save();
	if (!NVM && NVM != NVM_RAM) {
		munmap(NVM, MAPPED_MEM_SIZE);
		NVM = NULL;
	}
	return 0;
}

int ids_h6_restart(void)
{
	CompassLib_H6_ClearCalibration();
	return 0;
}

int ids_h6_get_state(void)
{
	int state = 0;
	if (NVM) {
		state = NVM[NVM_STATE_BYTE];
	}
	return state;
}

void ids_h6_clear_state(void)
{
	if (NVM) {
		NVM[NVM_STATE_BYTE] = 0;
	}
}

/*
 * Periodic task that runs at sensor sampling rate
 */
void ids_h6_data_in(struct SensorData_Algo *d)
{
	assert(d != NULL);
	CompassLib_H6_DataIn(d->gx, d->gy, d->gz, d->hx, d->hy, d->hz);
}

/*
 * Periodic task that runs at display update rate
 */
int ids_h6_calc_orientation(struct SensorData_Orientation *o, struct SensorData_Algo *d)
{
	uint8 isautocali, noiseLevel, calibrationQuality,Tquality;
	static COMPASSLIB_H6_ORIENTATION orientation;

#if FILTER
	static struct SensorData_Algo od;
#endif

	assert(d != NULL);
	assert(orien != NULL);

	orientation.Pitch = 0;
	orientation.Yaw = 0;
	orientation.Roll = 0;

	noiseLevel = CompassLib_H6_GetMagneticNoiseLevel();
	Tquality = CompassLib_H6_GetCalibrationQualityFactor();
	// convert calibration quality to orientation status
	if (Tquality <= 25) {
		calibrationQuality = SENSOR_STATUS_UNRELIABLE;
	} else if (Tquality <= 50) {
		calibrationQuality = SENSOR_STATUS_ACCURACY_LOW;
	} else if (Tquality <= 75) {
		calibrationQuality = SENSOR_STATUS_ACCURACY_MEDIUM;
	} else {
		calibrationQuality = SENSOR_STATUS_ACCURACY_HIGH;
	}

	isautocali = CompassLib_H6_IsAutoCalibrationEnabled();
	if (1){//((Tquality > 0) && (noiseLevel < 2)) {
	#if FILTER
		od.gx += ((d->gx - od.gx) >> 4);
		od.gy += ((d->gy - od.gy) >> 4);
		od.gz += ((d->gz - od.gz) >> 4);
		od.hx += ((d->hx - od.hx) >> 4);
		od.hy += ((d->hy - od.hy) >> 4);
		od.hz += ((d->hz - od.hz) >> 4);
		memcpy(d, &od, sizeof(od));
	#endif
		orientation = CompassLib_H6_CalculateOrientation(d->gx, d->gy, d->gz, d->hx, d->hy, d->hz);
		/* Convert orientation value from IDS definition to Android's */
		// Yaw
		if (d->gz > 0) {
			if (((d->gx/32768.0) < 0.5)&&((d->gx/32768.0) > -0.5)){
				orientation.Yaw =  - orientation.Yaw;
			}else{
				orientation.Yaw = 32768 - orientation.Yaw;
			}
		} else {
			orientation.Yaw = 65536 - orientation.Yaw;
		}
		// Pitch
		if (d->gz < 0) {
			orientation.Pitch = orientation.Pitch;		// y = -x
		} else {
			if (orientation.Pitch >= -65536/4 && orientation.Pitch < 0) {
				orientation.Pitch = -orientation.Pitch - 65536/2; // y = -x - 180
			} else {
				orientation.Pitch = -orientation.Pitch + 65536/2; // y = -x + 180
			}
		}
		// Roll
		if ((orientation.Roll >= -65536/2) && (orientation.Roll < -65536/4)) {
			orientation.Roll = orientation.Roll + 65536/2;	// -180 ~ -90: y = x + 180
		} else if (orientation.Roll >= -65536/4 && orientation.Roll < 0) {
			orientation.Roll = -orientation.Roll;		// -90 ~ 0 : y =  x
		} else if (orientation.Roll >= 0 && orientation.Roll < 65536/4) {
			orientation.Roll = -orientation.Roll;		// 0 ~ 90 : y =  x
		} else {
			orientation.Roll = orientation.Roll - 65536/2;	// 90 ~ 180: y = x - 180
		}
	

		o->azimuth = (uint16)orientation.Yaw;
		o->pitch = (int16)orientation.Pitch;
		o->roll = (int16)orientation.Roll;
	}
	#if SENSORS_ALGO_DEBUG
		LOGD("Acc:\t%2.3f\t%2.3f\t%2.3f\n"
			"Mag:\t%2.3f\t%2.3f\t%2.3f\n"
			"Orientation: R:%2.1f\t\tP:%2.1f\t\tY:%3.2f\n"
			"Quality:%1d\tNoise:%02d\n",
			d->gx/32768.0, d->gy/32768.0, d->gz/32768.0,
			d->hx/32768.0, d->hy/32768.0, d->hz/32768.0,
			ids_degree_to_angle(orientation.Roll),
			ids_degree_to_angle(orientation.Pitch),
			ids_degree_to_angle_positive(orientation.Yaw),
			calibrationQuality, noiseLevel);
	#endif
	o->noise_level = noiseLevel;
	o->quality = calibrationQuality;

	return 0;
}

/*
 * Continuous background loop for compass auto calibration
 */
int ids_h6_calibration(struct SensorData_Algo *d)
{
	COMPASSLIB_H6_STATE state;
	struct timespec tv_req, tv_rem;
	int n;

	assert(d != NULL);

	switch (CompassLib_H6_GetOperatingState())
	{
	case COMPASSLIB_H6_STATE_UNAVAILABLE:
	case COMPASSLIB_H6_STATE_OFF:
		// compass library requires no CPU time
		break;
			
	case COMPASSLIB_H6_STATE_ON:
	case COMPASSLIB_H6_STATE_BUSY:
		// compass library requires CPU time
		// task must be called at least once per second
		// should be called more often while busy
	#if 1	/* timeout to ensure auto-calibration not consume too much CPU time */
		for (n = 0; n < 5; n++) {
			// call the background task
			// pass in the current system clock
			// clock must count milliseconds, otherwise compass timing breaks 
			state = CompassLib_H6_Task(ids_get_milliseconds());
			if (state != COMPASSLIB_H6_STATE_BUSY)
				break; // no more CPU time needed			
		}
	#else	/* make sure auto-calibration well done no matter how much CPU time consume */
		do {
			state = CompassLib_H6_Task(ids_get_milliseconds());
		} while ((state == COMPASSLIB_H6_STATE_BUSY));
	#endif
		break;
	default:
		break;
	}
	return 0;
}

#if BUFFER_DUMP
void dump(const float v[], int size, int type, const char *str)
{
	int i;

	LOGD("Dump %s - %s:", str, type ? "MAX": "MIN");
        for (i = 0; i < size; i++) {
		LOGD(" %1.3f", v[i]);
	}
	LOGD("\n");
}
#else
#define dump(v, s, t, f)
#endif

static int float_max( const void *a , const void *b )
{
	return ((*(float *)a > *(float *)b) ? 1 : -1);
} 

static int float_min( const void *a , const void *b )
{
	return ((*(float *)a < *(float *)b) ? 1 : -1);
} 

#define magnetic_max_sort(v, size)	qsort((v), (size), sizeof((v)[0]), float_max)
#define magnetic_min_sort(v, size)	qsort((v), (size), sizeof((v)[0]), float_min)
#define magnetic_max_is_new(v, nd)	(((v)[0] == -MAG_MAX) || ((nd) > (v)[AVG_BUFF_SIZE-1]))
#define magnetic_min_is_new(v, nd)	(((v)[0] == MAG_MAX) || ((nd) < (v)[AVG_BUFF_SIZE-1]))
#define magnetic_max_insert(v, nm)	((v)[0] = (nm))	// replace the minimum in vector v
#define magnetic_min_insert(v, nm)	((v)[0] = (nm))	// replace the maximum in vector v

static int magnetic_max_is_valid(const float v[], float new_max)
{
	int i = 0;
	float diff_a = .0, delt = .0;

	if (v[0] == -MAG_MAX) {
		return 1;
	}

	delt = new_max - v[AVG_BUFF_SIZE-1];
	if (delt < 0) {
		return 0;	// invalid if negative delta
	}

	// differential sum
	for (i = 0; i < AVG_BUFF_SIZE - 1; i++) {
		diff_a += v[(AVG_BUFF_SIZE-i)-1] - v[(AVG_BUFF_SIZE-i)-2];
	}	
	// differential average
	diff_a /= (AVG_BUFF_SIZE - 1);

	return ((diff_a * MAG_NOI_THRESHOLD - delt) > 0.0);
}

static int magnetic_min_is_valid(const float v[], float new_min)
{
	int i = 0;
	float diff_a = .0, delt = .0;

	if (v[0] == MAG_MAX) {
		return 1;
	}

	delt = v[AVG_BUFF_SIZE-1] - new_min;
	if (delt < 0) {
		return 0;	// invalid if negative delta
	}

	// differential
	for (i = 0; i < AVG_BUFF_SIZE - 1; i++) {
		diff_a += v[i] - v[i+1];
	}	
	// differential average
	diff_a /= (AVG_BUFF_SIZE - 1);

	return ((diff_a * MAG_NOI_THRESHOLD - delt) > 0.0);
}

int ids_h6_calc_magcentre(const float mag[3], float centre[3])
{
	int i = 0;	
	float avg_max_x = .0, avg_min_x = .0;
	float avg_max_y = .0, avg_min_y = .0;
	float avg_max_z = .0, avg_min_z = .0;
	
	
	if (magnetic_max_is_new(mcb_max.vx, mag[0])) {
		if (magnetic_max_is_valid(mcb_max.vx, mag[0])) {
			magnetic_max_insert(mcb_max.vx, mag[0]);
			magnetic_max_sort(mcb_max.vx, AVG_BUFF_SIZE);
			dump(mcb_max.vx, AVG_BUFF_SIZE, 1, "X");
		}
	}
	if (magnetic_min_is_new(mcb_min.vx, mag[0])) {
		if (magnetic_min_is_valid(mcb_min.vx, mag[0])) {
			magnetic_min_insert(mcb_min.vx, mag[0]);
			magnetic_min_sort(mcb_min.vx, AVG_BUFF_SIZE);
			dump(mcb_min.vx, AVG_BUFF_SIZE, 0, "X");
		}
	}

	if (magnetic_max_is_new(mcb_max.vy, mag[1])) {
		if (magnetic_max_is_valid(mcb_max.vy, mag[1])) {
			magnetic_max_insert(mcb_max.vy, mag[1]);
			magnetic_max_sort(mcb_max.vy, AVG_BUFF_SIZE);
			dump(mcb_max.vy, AVG_BUFF_SIZE, 1, "Y");
		}
	}
	if (magnetic_min_is_new(mcb_min.vy, mag[1])) {
		if (magnetic_min_is_valid(mcb_min.vy, mag[1])) {
			magnetic_min_insert(mcb_min.vy, mag[1]);
			magnetic_min_sort(mcb_min.vy, AVG_BUFF_SIZE);
			dump(mcb_min.vy, AVG_BUFF_SIZE, 0, "Y");
		}
	}
	
	if (magnetic_max_is_new(mcb_max.vz, mag[2])) {
		if (magnetic_max_is_valid(mcb_max.vz, mag[2])) {
			magnetic_max_insert(mcb_max.vz, mag[2]);
			magnetic_max_sort(mcb_max.vz, AVG_BUFF_SIZE);
			dump(mcb_max.vz, AVG_BUFF_SIZE, 1, "Z");
		}
	}
	if (magnetic_min_is_new(mcb_min.vz, mag[2])) {
		if (magnetic_min_is_valid(mcb_min.vz, mag[2])) {
			magnetic_min_insert(mcb_min.vz, mag[2]);
			magnetic_min_sort(mcb_min.vz, AVG_BUFF_SIZE);
			dump(mcb_min.vz, AVG_BUFF_SIZE, 0, "Z");
		}
	}
	
	for (i = 0; i < AVG_BUFF_SIZE; i++) {
		avg_max_x += mcb_max.vx[i];
		avg_min_x += mcb_min.vx[i];

		avg_max_y += mcb_max.vy[i];
		avg_min_y += mcb_min.vy[i];

		avg_max_z += mcb_max.vz[i];
		avg_min_z += mcb_min.vz[i];
	}
	avg_max_x /= AVG_BUFF_SIZE;
	avg_min_x /= AVG_BUFF_SIZE;

	avg_max_y /= AVG_BUFF_SIZE;
	avg_min_y /= AVG_BUFF_SIZE;

	avg_max_z /= AVG_BUFF_SIZE;
	avg_min_z /= AVG_BUFF_SIZE;

	centre[0] = (avg_max_x + avg_min_x) / 2;
	centre[1] = (avg_max_y + avg_min_y) / 2;
	centre[2] = (avg_max_z + avg_min_z) / 2;

	return 0;
}
int ids_h6_calc_magcal_data(struct SensorData_Algo *d, float cald[3])
{
	Geomagnetic_Offset_Value Mag_Algo_Offset;
	assert(d != NULL);
	Mag_Algo_Offset = CompassLib_H6_Get_Offset();
	cald[0] = ((uint16)(d->hy >> 3) - Mag_Algo_Offset.x) / 512.0;
	cald[1] = ((uint16)(d->hx >> 3) - Mag_Algo_Offset.y) / 512.0;
	cald[2] = ((uint16)((0xffff-d->hz) >> 3) - Mag_Algo_Offset.z) / 512.0;

	return 0;
}
#endif /* COMPASS_ALGO_H6 */

