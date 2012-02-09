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
 * This file implement acceleration sensor adapter for BMA220.
 */
#if (defined DEVICE_ACC_BMA220)

#define LOG_TAG "SensorBMA220"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/ipc.h>
#include <sys/mman.h>

#include <sensors_acc_bma220.h>

#ifdef MODULE_DEBUG
#include <utils/Log.h>
#endif

#define SENSORS_ACC_DEBUG	1

#if (!defined MODULE_DEBUG)
#if SENSORS_ACC_DEBUG
#define LOGD(x...)		printf(x)
#define LOGE(x...)		printf(x)
#else
#define LOGD(x...)
#define LOGE(x...)
#endif
#endif

#define BMA220_OFFSET_X			0
#define BMA220_OFFSET_Y			0
#define BMA220_OFFSET_Z			0
#define BMA220_SENSITIVITY_X		16
#define BMA220_SENSITIVITY_Y		16
#define BMA220_SENSITIVITY_Z		16

/**
 * NOTE:
 * You are required to get the correct install direction according 
 * the sensor placement on target board 
 */
#define BMA220_INSTALL_DIR		4

/*----------------------------------------------------------*/
// include header for sensor BMA220 here
/*----------------------------------------------------------*/

/*----------------------------------------------------------*/
#define BMA220_IOC_MAGIC 'B'
#define BMA220_READ_ACCEL_XYZ		_IOWR(BMA220_IOC_MAGIC,26,signed char)
#define BMA220_GET_OFFSET_XYZ		_IOWR(BMA220_IOC_MAGIC,91,signed char)
/*----------------------------------------------------------*/

#define OFFSET_PATH	"/data/misc/sensors/accel_offset"

#define MAPPED_MEM_SIZE	256

static int offset_nvm[3];

static char *mapped_mem = NULL;
static char buffer_ram[MAPPED_MEM_SIZE];

int acc_bma220_init(void)
{
	int fd = -1;
	int n;

	fd = open(OFFSET_PATH, O_RDWR | O_CREAT, 0666);
	if (fd == -1) {
		LOGE("%s: fail to open %s\n", __FUNCTION__, OFFSET_PATH);
		// set offset to all zero default
		offset_nvm[0] = BMA220_OFFSET_X;
		offset_nvm[1] = BMA220_OFFSET_Y;
		offset_nvm[2] = BMA220_OFFSET_Z;

		return -1;
	}

	n = read(fd, buffer_ram, sizeof(buffer_ram));
	if (n < 9) {	// strlen("x:X,y:Y,z:Z") >= 9
		// set offset to all zero default
		offset_nvm[0] = BMA220_OFFSET_X;
		offset_nvm[1] = BMA220_OFFSET_Y;
		offset_nvm[2] = BMA220_OFFSET_Z;

		sprintf(buffer_ram, "x:%04d,y:%04d,z:%04d\n", 
			offset_nvm[0], offset_nvm[1], offset_nvm[2]);
		n = write(fd, buffer_ram, strlen(buffer_ram));
	} else {
		// load accel offset from param file
		sscanf(buffer_ram, "x:%d,y:%d,z:%d", 
			&offset_nvm[0], &offset_nvm[1], &offset_nvm[2]);
	}

	mapped_mem = mmap(0, MAPPED_MEM_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
	if (mapped_mem == MAP_FAILED) {
		// set NVM to RAM if failure
		mapped_mem = buffer_ram;
	}

	close(fd);

	return 0;
}

int acc_bma220_open(void)
{
	int fd;

	fd = open("/dev/bma220", O_RDWR);
	if (fd < 0) {
		return -1;
	}

	return fd;
}

int acc_bma220_close(int fd)
{
	if (mapped_mem != buffer_ram) {
		munmap(mapped_mem, MAPPED_MEM_SIZE);
	}
	close(fd);

	return 0;
}

int acc_bma220_read_data(int fd, int *data)
{
	signed char acc[3] = {0};

	assert(data != NULL);

#if 0   // TEST
	/*----------------------------------------------------*/
	// this block is for simulating test,
	// please use the #else block in your real target
	/*----------------------------------------------------*/

	// pseudo data here to simulate Bosch BMA200 sensor
	// range	: -2g ~ +2g
	// sensitivity	: 16
	// offset	: 0
	acc[0] = 1;
	acc[1] = 15;
	acc[2] = 2;
#else
	if (ioctl(fd, BMA220_READ_ACCEL_XYZ, acc)) {
		LOGE("BMA_READ_ACCEL_XYZ fail");
		return -1;
	}
#endif

	data[0] = acc[0];
	data[1] = acc[1];
	data[2] = acc[2];

	return 0;
}

int acc_bma220_get_offset(int fd, int *offset_xyz)
{
	signed char offset[3] = {0};

	assert(offset_xyz != NULL);

#if 1   // use the default offset defined in sensor spec
	/*----------------------------------------------------*/
	// this block assume accelerometer's offset is calibrated already,
	// so that we just use the default offset defined in sensor spec
	/*----------------------------------------------------*/

	// const value '0' for BMA220
	offset[0] = BMA220_OFFSET_X;
	offset[1] = BMA220_OFFSET_Y;
	offset[2] = BMA220_OFFSET_Z;
#else	// get offset from vendor sensor config or from sensor device
	/*----------------------------------------------------*/
	// this block get accelerometer's offset from sensor's eeprom,
	/*----------------------------------------------------*/
	if (ioctl(fd, BMA220_GET_OFFSET_XYZ, offset)) {
		LOGE("BMA220_GET_OFFSET_XYZ fail");
		return -1;
	}
#endif

	sscanf(mapped_mem, "x:%d,y:%d,z:%d", 
		&offset_nvm[0], &offset_nvm[1], &offset_nvm[2]);

	offset_xyz[0] = offset_nvm[0];
	offset_xyz[1] = offset_nvm[1];
	offset_xyz[2] = offset_nvm[2];

	return 0;
}

int acc_bma220_set_new_offset(int fd, int *offset_xyz)
{
	int fd_parm = -1;
	int n;

	offset_nvm[0] = offset_xyz[0];
	offset_nvm[1] = offset_xyz[1];
	offset_nvm[2] = offset_xyz[2];

	// store accel new offset to param file
	sprintf(mapped_mem, "x:%04d,y:%04d,z:%04d\n", 
		offset_nvm[0], offset_nvm[1], offset_nvm[2]);

	return 0;
}

int acc_bma220_get_sensitivity(int fd, int *sensit_xyz)
{
	// FIXME:
	// should get sensivitity from vendor sensor config or from sensor device
	// const value '16' for BMA220 tmp
	assert(sensit_xyz != NULL);

	sensit_xyz[0] = BMA220_SENSITIVITY_X;
	sensit_xyz[1] = BMA220_SENSITIVITY_Y;
	sensit_xyz[2] = BMA220_SENSITIVITY_Z;

	return 0;
}

int acc_bma220_get_install_dir(void)
{
	return BMA220_INSTALL_DIR;
}

#endif /* DEVICE_ACC_BMA220 */

