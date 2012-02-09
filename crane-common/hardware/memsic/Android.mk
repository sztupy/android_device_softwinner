ifeq ($(BOARD_USES_GSENSOR_TYPE), mxc622x)

LOCAL_PATH := $(call my-dir)

# prebuilt algorithm libs
include $(CLEAR_VARS)
LOCAL_PREBUILT_LIBS := compasslib_h5_gcc_armv4t.a compasslib_h6_gcc_armv4t.a
LOCAL_MODULE_TAGS := eng
include $(BUILD_MULTI_PREBUILT)

# MEMSIC sensors daemon
include $(CLEAR_VARS)
LOCAL_PRELINK_MODULE	:= false
LOCAL_SHARED_LIBRARIES	:= 
LOCAL_STATIC_LIBRARIES	:= compasslib_h5_gcc_armv4t compasslib_h6_gcc_armv4t
LOCAL_LDLIBS		+= -Idl
LOCAL_CFLAGS		+= -static
LOCAL_MODULE_TAGS := eng
# use the following macro to specify acc/mag sensor and orientation algorithm
# Accelerometer:
# DEVICE_ACC_MXC6202XG - MEMSIC MXC6202xG 2-axis acc-sensor
# DEVICE_ACC_MXC6202XH - MEMSIC MXC6202xH 2-axis acc-sensor
# DEVICE_ACC_MXC6202XM - MEMSIC MXC6202xM 2-axis acc-sensor
# DEVICE_ACC_MXC6202XN - MEMSIC MXC6202xN 2-axis acc-sensor
# DEVICE_ACC_MXC622X   - MEMSIC MXC622x (DTOS) 2-axis acc-sensor
# DEVICE_ACC_BMA220    - BOSCH BMA220
# DEVICE_ACC_BMA023    - BOSCH BMA023
# Magnetic sensor:
# DEVICE_MAG_MMC312X   - MEMSIC MMC3120 3-axis mag-sensor
# DEVICE_MAG_MMC314X   - MEMSIC MMC3140 3-axis mag-sensor
# DEVICE_MAG_MMC328X   - MEMSIC MMC328X 3-axis mag-sensor
# Orientation algorithm:
# COMPASS_ALGO_H5      - IDS 3+2 algorithm
# COMPASS_ALGO_H6      - IDS 3+3 algorithm
#LOCAL_CFLAGS		+= -DDEVICE_ACC_LIS33DE -DDEVICE_MAG_MMC328X -DCOMPASS_ALGO_H6
LOCAL_CFLAGS		+= -DDEVICE_ACC_MXC622X -DDEVICE_MAG_MMC312X -DCOMPASS_ALGO_H5
LOCAL_C_INCLUDES	+= $(LOCAL_PATH)/adapter		\
			   $(LOCAL_PATH)/libs/H5_V1.2		\
			   $(LOCAL_PATH)/libs/H6_V1.0

LOCAL_SRC_FILES		:= daemon/memsicd.c			\
			   adapter/sensors_coordinate.c		\
			   adapter/sensors_acc_adapter.c    	\
			   adapter/sensors_acc_mxc6202x.c    	\
			   adapter/sensors_acc_mxc622x.c    	\
			   adapter/sensors_acc_bma220.c    	\
			   adapter/sensors_acc_bma023.c    	\
			   adapter/sensors_acc_lis33de.c    	\
			   adapter/sensors_mag_adapter.c    	\
			   adapter/sensors_mag_mmc31xx.c    	\
			   adapter/sensors_mag_mmc328x.c    	\
			   adapter/sensors_algo_adapter.c	\
			   adapter/sensors_algo_ids_h5.c	\
			   adapter/sensors_algo_ids_h6.c	\
			   adapter/sensors_algo_ids_util.c

LOCAL_MODULE := memsicd
include $(BUILD_EXECUTABLE)

# HAL module implemenation, not prelinked and stored in
# hw/<SENSORS_HARDWARE_MODULE_ID>.<ro.product.board>.so
include $(CLEAR_VARS)
LOCAL_PRELINK_MODULE	:= false
LOCAL_MODULE_TAGS := eng
LOCAL_SHARED_LIBRARIES	:= liblog libcutils
LOCAL_STATIC_LIBRARIES	:= 
LOCAL_MODULE_PATH	:= $(TARGET_OUT_SHARED_LIBRARIES)/hw
# use the following macro to specify acc/mag sensor and orientation algorithm
# Accelerometer:
# DEVICE_ACC_MXC6202XG - MEMSIC MXC6202xG 2-axis acc-sensor
# DEVICE_ACC_MXC6202XH - MEMSIC MXC6202xH 2-axis acc-sensor
# DEVICE_ACC_MXC6202XM - MEMSIC MXC6202xM 2-axis acc-sensor
# DEVICE_ACC_MXC6202XN - MEMSIC MXC6202xN 2-axis acc-sensor
# DEVICE_ACC_MXC622X   - MEMSIC MXC622x (DTOS) 2-axis acc-sensor
# DEVICE_ACC_BMA220    - BOSCH BMA220
# DEVICE_ACC_BMA023    - BOSCH BMA023
# Magnetic sensor:
# DEVICE_MAG_MMC312X   - MEMSIC MMC3120 3-axis mag-sensor
# DEVICE_MAG_MMC314X   - MEMSIC MMC3140 3-axis mag-sensor
# Orientation algorithm:
# COMPASS_ALGO_H5      - IDS 3+2 algorithm
# COMPASS_ALGO_H6      - IDS 3+3 algorithm
#LOCAL_CFLAGS		+= -DDEVICE_ACC_LIS33DE -DDEVICE_MAG_MMC328X -DCOMPASS_ALGO_H6
LOCAL_CFLAGS		+= -DDEVICE_ACC_MXC622X -DDEVICE_MAG_MMC312X -DCOMPASS_ALGO_H5
LOCAL_C_INCLUDES	+= 


ifeq ($(BOARD_GSENSOR_DIRECT_X), true)
LOCAL_CFLAGS += -DGSENSOR_DIRECT_X
endif

ifeq ($(BOARD_GSENSOR_DIRECT_Y), true)
LOCAL_CFLAGS += -DGSENSOR_DIRECT_Y
endif

ifeq ($(BOARD_GSENSOR_DIRECT_Z), true)
LOCAL_CFLAGS += -DGSENSOR_DIRECT_Z
endif

ifeq ($(BOARD_GSENSOR_XY_REVERT), true)
LOCAL_CFLAGS += -DGSENSOR_XY_REVERT
endif

LOCAL_SRC_FILES		:= hal/sensors_hal.c

#LOCAL_MODULE := sensors.softwinner
LOCAL_MODULE := sensors.$(TARGET_BOARD_PLATFORM)
include $(BUILD_SHARED_LIBRARY)

# Calibration module
include $(CLEAR_VARS)
LOCAL_PRELINK_MODULE	:= false
LOCAL_MODULE_TAGS := eng
LOCAL_SHARED_LIBRARIES	:= liblog libcutils
LOCAL_STATIC_LIBRARIES	:= compasslib_h5_gcc_armv4t compasslib_h6_gcc_armv4t
LOCAL_MODULE_PATH	:= $(TARGET_OUT_SHARED_LIBRARIES)
# use the following macro to specify acc sensor
# Accelerometer:
# DEVICE_ACC_MXC6202XG - MEMSIC MXC6202xG 2-axis acc-sensor
# DEVICE_ACC_MXC6202XH - MEMSIC MXC6202xH 2-axis acc-sensor
# DEVICE_ACC_MXC6202XM - MEMSIC MXC6202xM 2-axis acc-sensor
# DEVICE_ACC_MXC6202XN - MEMSIC MXC6202xN 2-axis acc-sensor
# DEVICE_ACC_BMA220    - BOSCH BMA220
# DEVICE_ACC_BMA023    - BOSCH BMA023
# Magnetic sensor:
# DEVICE_MAG_MMC312X   - MEMSIC MMC3120 3-axis mag-sensor
# DEVICE_MAG_MMC314X   - MEMSIC MMC3140 3-axis mag-sensor
# Orientation algorithm:
# COMPASS_ALGO_H5      - IDS 3+2 algorithm
# COMPASS_ALGO_H6      - IDS 3+3 algorithm
#LOCAL_CFLAGS		+= -DDEVICE_ACC_BMA220 -DCOMPASS_ALGO_H6
LOCAL_CFLAGS		+= -DDEVICE_ACC_MXC622X -DDEVICE_MAG_MMC312X -DCOMPASS_ALGO_H5
LOCAL_C_INCLUDES	+= $(LOCAL_PATH)/adapter		\
			   $(LOCAL_PATH)/libs/H5_V1.2		\
			   $(LOCAL_PATH)/libs/H6_V1.0

LOCAL_SRC_FILES		:= cal/accel_cal.c			\
			   adapter/sensors_acc_adapter.c    	\
			   adapter/sensors_acc_mxc6202x.c    	\
			   adapter/sensors_acc_mxc622x.c    	\
			   adapter/sensors_acc_bma220.c    	\
			   adapter/sensors_acc_bma023.c		\
			   adapter/sensors_algo_adapter.c	\
			   adapter/sensors_algo_ids_h5.c	\
			   adapter/sensors_algo_ids_h6.c	\
			   adapter/sensors_algo_ids_util.c

LOCAL_MODULE := libaccelcal
include $(BUILD_SHARED_LIBRARY)

endif