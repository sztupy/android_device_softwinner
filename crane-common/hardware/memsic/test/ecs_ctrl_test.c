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
#include <unistd.h>


/* Use 'e' as magic number */
#define ECOMPASS_IOM			'e'

/* IOCTLs for ECOMPASS device */
#define ECOMPASS_IOC_SET_MODE		_IOW(ECOMPASS_IOM, 0x00, short)
#define ECOMPASS_IOC_SET_DELAY		_IOW(ECOMPASS_IOM, 0x01, short)
#define ECOMPASS_IOC_GET_DELAY		_IOR(ECOMPASS_IOM, 0x02, short)

#define ECOMPASS_IOC_SET_AFLAG		_IOW(ECOMPASS_IOM, 0x10, short)
#define ECOMPASS_IOC_GET_AFLAG		_IOR(ECOMPASS_IOM, 0x11, short)
#define ECOMPASS_IOC_SET_MFLAG		_IOW(ECOMPASS_IOM, 0x12, short)
#define ECOMPASS_IOC_GET_MFLAG		_IOR(ECOMPASS_IOM, 0x13, short)
#define ECOMPASS_IOC_SET_OFLAG		_IOW(ECOMPASS_IOM, 0x14, short)
#define ECOMPASS_IOC_GET_OFLAG		_IOR(ECOMPASS_IOM, 0x15, short)

#define ECOMPASS_IOC_SET_APARMS		_IOW(ECOMPASS_IOM, 0x20, int[4])
#define ECOMPASS_IOC_GET_APARMS		_IOR(ECOMPASS_IOM, 0x21, int[4])
#define ECOMPASS_IOC_SET_MPARMS		_IOW(ECOMPASS_IOM, 0x22, int[4])
#define ECOMPASS_IOC_GET_MPARMS		_IOR(ECOMPASS_IOM, 0x23, int[4])
#define ECOMPASS_IOC_SET_OPARMS_YAW	_IOW(ECOMPASS_IOM, 0x24, int[4])
#define ECOMPASS_IOC_GET_OPARMS_YAW	_IOR(ECOMPASS_IOM, 0x25, int[4])
#define ECOMPASS_IOC_SET_OPARMS_PITCH	_IOW(ECOMPASS_IOM, 0x26, int[4])
#define ECOMPASS_IOC_GET_OPARMS_PITCH	_IOR(ECOMPASS_IOM, 0x27, int[4])
#define ECOMPASS_IOC_SET_OPARMS_ROLL	_IOW(ECOMPASS_IOM, 0x28, int[4])
#define ECOMPASS_IOC_GET_OPARMS_ROLL	_IOR(ECOMPASS_IOM, 0x29, int[4])

#define ECOMPASS_IOC_SET_YPR		_IOW(ECOMPASS_IOM, 0x30, int[12])


static int open_ctrl_dev(int mode)
{
	int fd;

	fd = open("/dev/ecompass_ctrl", mode);

	return fd;
}


int main(void)
{
	int fd;
	int flag = 1;
	int delay;
	int parms[4];
	int val[12];
	int tmp = 0;
	int i;
	int res;

	fd = open_ctrl_dev(O_RDWR);
	printf("ecs_ctrl fd: %d\n", fd);
	if (fd < 0) {
		printf("ecs_ctrl open failed\n");
		return -1;
	}
#if 0
	parms[0] = 2048 - 512 * 2;
	parms[1] = 2048 + 512 * 2;
	parms[2] = 0;
	parms[3] = 0;
	res = ioctl(fd, ECOMPASS_IOC_SET_APARMS, parms);
	printf("ECOMPASS_IOC_SET_APARMS ioctl res: %d\n", res);
	parms[1] = 2048 - 512 * 2;
	parms[2] = 2048 + 512 * 2;
	parms[3] = 0;
	parms[4] = 0;
	res = ioctl(fd, ECOMPASS_IOC_SET_MPARMS, parms);
	printf("ECOMPASS_IOC_SET_MPARMS ioctl res: %d\n", res);
	parms[0] = 0;
	parms[1] = 360;
	parms[2] = 0;
	parms[3] = 0;
	res = ioctl(fd, ECOMPASS_IOC_SET_OPARMS_YAW, parms);
	printf("ECOMPASS_IOC_SET_OPARMS_YAW ioctl res: %d\n", res);
	parms[0] = -180;
	parms[1] = 180;
	parms[2] = 0;
	parms[3] = 0;
	res = ioctl(fd, ECOMPASS_IOC_SET_OPARMS_PITCH, parms);
	printf("ECOMPASS_IOC_SET_OPARMS_PITCH ioctl res: %d\n", res);
	parms[0] = -90;
	parms[1] = 90;
	parms[2] = 0;
	parms[3] = 0;
	res = ioctl(fd, ECOMPASS_IOC_SET_OPARMS_ROLL, parms);
	printf("ECOMPASS_IOC_SET_OPARMS_ROLL ioctl res: %d\n", res);
#endif

	res = ioctl(fd, ECOMPASS_IOC_GET_AFLAG, &flag);
	printf("ECOMPASS_IOC_GET_AFLAG ioctl res: %d, flag: %d\n", res, flag);
	res = ioctl(fd, ECOMPASS_IOC_GET_MFLAG, &flag);
	printf("ECOMPASS_IOC_GET_MFLAG ioctl res: %d, flag: %d\n", res, flag);
	res = ioctl(fd, ECOMPASS_IOC_GET_OFLAG, &flag);
	printf("ECOMPASS_IOC_GET_OFLAG ioctl res: %d, flag: %d\n", res, flag);

	flag = 1;
	res = ioctl(fd, ECOMPASS_IOC_SET_AFLAG, &flag);
	printf("ECOMPASS_IOC_SET_AFLAG ioctl res: %d\n", res);
	res = ioctl(fd, ECOMPASS_IOC_SET_MFLAG, &flag);
	printf("ECOMPASS_IOC_SET_MFLAG ioctl res: %d\n", res);
	res = ioctl(fd, ECOMPASS_IOC_SET_OFLAG, &flag);
	printf("ECOMPASS_IOC_SET_OFLAG ioctl res: %d\n", res);

	res = ioctl(fd, ECOMPASS_IOC_GET_AFLAG, &flag);
	printf("ECOMPASS_IOC_GET_AFLAG ioctl res: %d, flag: %d\n", res, flag);
	res = ioctl(fd, ECOMPASS_IOC_GET_MFLAG, &flag);
	printf("ECOMPASS_IOC_GET_MFLAG ioctl res: %d, flag: %d\n", res, flag);
	res = ioctl(fd, ECOMPASS_IOC_GET_OFLAG, &flag);
	printf("ECOMPASS_IOC_GET_OFLAG ioctl res: %d, flag: %d\n", res, flag);

	while (1) {
		tmp++;
		for (i = 0; i < 12; i++) {
			val[i] = tmp;
		}
		res = ioctl(fd, ECOMPASS_IOC_SET_YPR, val);
		printf("ECOMPASS_IOC_SET_YPR ioctl res: %d\n", res);
		printf("Set YPR:\n");
		printf("Accel:\t[%04d] [%04d] [%04d] [%04d]\n", 
			val[0], val[1], val[2], val[3]);
		printf("MagV:\t[%04d] [%04d] [%04d] [%04d]\n", 
			val[4], val[5], val[6], val[7]);
		printf("Orient:\t[%04d] [%04d] [%04d] [%04d]\n", 
			val[8], val[9], val[10], val[11]);

		sleep(2);
	}

	close(fd);

	return 0;
}
