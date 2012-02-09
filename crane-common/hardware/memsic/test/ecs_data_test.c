/*****************************************************************************
 *  Copyright Statement:
 *  --------------------
 *  This software is protected by Copyright and the information and source code
 *  contained herein is confidential. The software including the source code
 *  may not be copied and the information contained herein may not be used or
 *  disclosed except with the written permission of MEMSIC Inc. (C) 2010
 *****************************************************************************/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/ipc.h>
#include <sys/time.h>
#include <time.h>
#include <unistd.h>
#include <dirent.h>
#include <math.h>
#include <poll.h>

#include <linux/input.h>

#define EVENT_TYPE_ACCEL_X		ABS_X
#define EVENT_TYPE_ACCEL_Y		ABS_Y
#define EVENT_TYPE_ACCEL_Z		ABS_Z
#define EVENT_TYPE_ACCEL_STATUS		ABS_WHEEL

#define EVENT_TYPE_MAGV_X		ABS_HAT0X
#define EVENT_TYPE_MAGV_Y		ABS_HAT0Y
#define EVENT_TYPE_MAGV_Z		ABS_BRAKE
#define EVENT_TYPE_MAGV_STATUS		ABS_GAS

#define EVENT_TYPE_ORIENT_YAW		ABS_RX
#define EVENT_TYPE_ORIENT_PITCH		ABS_RY
#define EVENT_TYPE_ORIENT_ROLL		ABS_RZ
#define EVENT_TYPE_ORIENT_STATUS	ABS_RUDDER


static int open_input(int mode)
{
	/* scan all input drivers and look for "compass" */
	int fd = -1;
	const char *dirname = "/dev/input";
	char devname[PATH_MAX];
	char *filename;
	DIR *dir;
	struct dirent *de;
	dir = opendir(dirname);
	if(dir == NULL) {
		return -1;
	}
	strcpy(devname, dirname);
	filename = devname + strlen(devname);
	*filename++ = '/';
	while((de = readdir(dir))) {
		if(de->d_name[0] == '.' && (de->d_name[1] == '\0' ||
			(de->d_name[1] == '.' && de->d_name[2] == '\0'))) {
			continue;
		}
		strcpy(filename, de->d_name);
		fd = open(devname, mode);
		if (fd >= 0) {
			char name[80];
			if (ioctl(fd, EVIOCGNAME(sizeof(name) - 1), &name) < 1) {
				name[0] = '\0';
			}
			if (!strcmp(name, "ecompass_data")) {
				printf("using %s (name=%s)\n", devname, name);
				break;
			}
			close(fd);
			fd = -1;
		}
	}
	closedir(dir);

	if (fd < 0) {
		printf("Couldn't find or open 'compass' driver (%s)\n", 
			strerror(errno));
	}

	return fd;
}


int main(void)
{
	int i;
	int fd;
	int val[12];

	fd = open_input(O_RDONLY);
	printf("ecs_data fd: %d\n", fd);

	while (1) {
		/* read the next event */
		struct input_event event;
		int nread = read(fd, &event, sizeof(event));
		memset(val, 0, sizeof(val));
		if (nread == sizeof(event)) {
			uint32_t v;
			if (event.type == EV_ABS) {
				//LOGD("type: %d code: %d value: %-5d time: %ds",
				//		event.type, event.code, event.value,
				//	  (int)event.time.tv_sec);
				switch (event.code) {
					case EVENT_TYPE_ACCEL_X:
						val[0] = event.value;
						break;
					case EVENT_TYPE_ACCEL_Y:
						val[1] = event.value;
						break;
					case EVENT_TYPE_ACCEL_Z:
						val[2] = event.value;
						break;
					case EVENT_TYPE_ACCEL_STATUS:
						// accuracy of the calibration (never returned!)
						//LOGD("G-Sensor status %d", event.value);
						break;

					case EVENT_TYPE_MAGV_X:
						val[4] = event.value;
						break;
					case EVENT_TYPE_MAGV_Y:
						val[5] = event.value;
						break;
					case EVENT_TYPE_MAGV_Z:
						val[6] = event.value;
						break;
					case EVENT_TYPE_MAGV_STATUS:
						// accuracy of the calibration (never returned!)
						//LOGD("G-Sensor status %d", event.value);
						break;

					case EVENT_TYPE_ORIENT_YAW:
						val[8] = event.value;
						break;
					case EVENT_TYPE_ORIENT_PITCH:
						val[9] = event.value;
						break;
					case EVENT_TYPE_ORIENT_ROLL:
						val[10] = event.value;
						break;
					case EVENT_TYPE_ORIENT_STATUS:
						// accuracy of the calibration
						break;
				}
			} else if (event.type == EV_SYN) {
				if (event.code == SYN_CONFIG) {
					// we use SYN_CONFIG to signal that we need to exit the
					// main loop.
					//LOGD("got empty message: value=%d", event.value);
					continue;
				}
			}
			printf("input_event: type %d, code %d value %d\n", 
				event.type, event.code, event.value);
		}
		usleep(100);
	}

	close(fd);

	return 0;
}
