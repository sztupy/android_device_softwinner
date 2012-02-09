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
#define MMC328X_IOM			'm'

/* IOCTLs for MMC328X device */
#define MMC328X_IOC_TM			_IO (MMC328X_IOM, 0x00)
#define MMC328X_IOC_RM			_IO (MMC328X_IOM, 0x01)
#define MMC328X_IOC_READ		_IOR(MMC328X_IOM, 0x02, int[3])
#define MMC328X_IOC_READXYZ		_IOR(MMC328X_IOM, 0x03, int[3])


int main()
{
	int fd;
	int result;
	int mag[3];

	fd = open("/dev/mmc328x", O_RDWR);
	for (;;) {
		printf("********************************************\n");
		result = ioctl(fd, MMC328X_IOC_READXYZ, mag);
		printf("Read result: %d\n", result);
		printf("[X - %04d] [Y - %04d] [Z - %04d]\n", 
			mag[0], mag[1], mag[2]);
		//sleep(1);
	}
	close(fd);

	return 0;
}

