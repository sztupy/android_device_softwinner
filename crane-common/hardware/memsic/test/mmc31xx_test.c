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
#define MMC31XX_IOM			'm'

/* IOCTLs for MMC31XX device */
#define MMC31XX_IOC_TM			_IO (MMC31XX_IOM, 0x00)
#define MMC31XX_IOC_SET			_IO (MMC31XX_IOM, 0x01)
#define MMC31XX_IOC_RESET		_IO (MMC31XX_IOM, 0x02)
#define MMC31XX_IOC_READ		_IOR(MMC31XX_IOM, 0x03, int[3])
#define MMC31XX_IOC_READXYZ		_IOR(MMC31XX_IOM, 0x04, int[3])

int main()
{
	int fd;
	int result;
	int mag[3];

	fd = open("/dev/mmc31xx", O_RDWR);
	for (;;) {
		printf("********************************************\n");
		result = ioctl(fd, MMC31XX_IOC_READXYZ, mag);
		printf("Read result: %d\n", result);
		printf("[X - %04x] [Y - %04x] [Z - %04x]\n", 
			mag[0], mag[1], mag[2]);
		//sleep(1);
	}
	close(fd);

	return 0;
}

