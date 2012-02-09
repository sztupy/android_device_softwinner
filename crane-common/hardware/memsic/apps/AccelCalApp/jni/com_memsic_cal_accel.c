/*****************************************************************************
 *  Copyright Statement:
 *  --------------------
 *  This software is protected by Copyright and the information and source code
 *  contained herein is confidential. The software including the source code
 *  may not be copied and the information contained herein may not be used or
 *  disclosed except with the written permission of MEMSIC Inc. (C) 2009
 *****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <dlfcn.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <dlfcn.h>

#include <jni.h>

jint
Java_com_memsic_cal_AccelCalApp_calibrate(JNIEnv*  env,
                                          jobject  this)
{
    void *handle;
    int  (*accelcal)(void);
    int  val = 0;

    handle = dlopen("libaccelcal.so", RTLD_NOW);
    if (handle == NULL) {
        fprintf(stderr, "dlopen failed: %s\n", dlerror());
        return -1;
    }
    accelcal = dlsym(handle, "accel_calibrate");
    val = accelcal();
    dlclose(handle);

    return (val);
}

