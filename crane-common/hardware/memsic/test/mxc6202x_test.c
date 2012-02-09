/*****************************************************************************
 *  Copyright Statement:
 *  --------------------
 *  This software is protected by Copyright and the information and source code
 *  contained herein is confidential. The software including the source code
 *  may not be copied and the information contained herein may not be used or
 *  disclosed except with the written permission of MEMSIC Inc. (C) 2009
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

/* Use 'm' as magic number */
#define MXC6202X_IOM			'm'

/* IOCTLs for MXC6202X device */
#define MXC6202X_IOC_PWRON		_IO (MXC6202X_IOM, 0x00)
#define MXC6202X_IOC_PWRDN		_IO (MXC6202X_IOM, 0x01)
#define MXC6202X_IOC_ST			_IO (MXC6202X_IOM, 0x02)
#define MXC6202X_IOC_BGTST		_IO (MXC6202X_IOM, 0x03)
#define MXC6202X_IOC_TOEN		_IO (MXC6202X_IOM, 0x04)
#define MXC6202X_IOC_READXYZ		_IOR(MXC6202X_IOM, 0x05, int[3])
#define MXC6202X_IOC_READTEMP		_IOR(MXC6202X_IOM, 0x06, int)

int main()
{
	int fd;
	int result;
	int acc[3];

	fd = open("/dev/mxc6202x", O_RDWR);
	result = ioctl(fd, MXC6202X_IOC_PWRON);
	sleep(1);
	for (;;) {
		printf("********************************************\n");
		result = ioctl(fd, MXC6202X_IOC_READXYZ, acc);
		printf("Read result: %d\n", result);
		printf("[X - %04x] [Y - %04x] [Z - %04x]\n", 
			acc[0], acc[1], acc[2]);
		sleep(1);
	}
	close(fd);

	return 0;
}

