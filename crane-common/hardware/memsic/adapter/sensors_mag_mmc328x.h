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
 *          Dale Hou<byhou@memsic.cn>
 * @brief
 * This file define the interface of magnetic sensor adapter for MMC328x.
 */

#ifndef __SENSORS_MAG_MMC328X_H__
#define __SENSORS_MAG_MMC328X_H__

int mag_mmc328x_init(void);

int mag_mmc328x_open(void);
int mag_mmc328x_close(int fd);

int mag_mmc328x_read_data(int fd, int *data);
int mag_mmc328x_get_offset(int fd, int *offset_xyz);
int mag_mmc328x_get_sensitivity(int fd, int *sensit_xyz);
int mag_mmc328x_get_install_dir(void);

#endif /* __SENSORS_MAG_MMC328X_H__ */

