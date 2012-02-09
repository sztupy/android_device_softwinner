# Copyright (C) 2008 The Android Open Source Project
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

ifneq ($(findstring $(BOARD_USES_GSENSOR_TYPE),mma7660 mma8451), )


LOCAL_PATH := $(call my-dir)

# HAL module implemenation, not prelinked, and stored in
# hw/<SENSORS_HARDWARE_MODULE_ID>.<ro.product.board>.so

include $(CLEAR_VARS)

ifeq ($(BOARD_USES_GSENSOR_TYPE), mma7660)
LOCAL_CPPFLAGS += -DACCELEROMETER_SENSOR_MMA7660
endif

ifeq ($(BOARD_USES_GSENSOR_TYPE), mma8451)
LOCAL_CPPFLAGS += -DACCELEROMETER_SENSOR_MMA8451
endif


ifeq ($(BOARD_GSENSOR_DIRECT_X), true)
LOCAL_CPPFLAGS += -DGSENSOR_DIRECT_X
endif

ifeq ($(BOARD_GSENSOR_DIRECT_Y), true)
LOCAL_CPPFLAGS += -DGSENSOR_DIRECT_Y
endif

ifeq ($(BOARD_GSENSOR_DIRECT_Z), true)
LOCAL_CPPFLAGS += -DGSENSOR_DIRECT_Z
endif

ifeq ($(BOARD_GSENSOR_XY_REVERT), true)
LOCAL_CPPFLAGS += -DGSENSOR_XY_REVERT
endif

LOCAL_PRELINK_MODULE := false
LOCAL_MODULE_PATH := $(TARGET_OUT_SHARED_LIBRARIES)/hw
LOCAL_MODULE := sensors.$(TARGET_BOARD_PLATFORM)
LOCAL_MODULE_TAGS := eng
LOCAL_CFLAGS := -DLOG_TAG=\"Sensors\"

LOCAL_SRC_FILES := 						\
				sensors.cpp 			\
				SensorBase.cpp			\
				LightSensor.cpp			\
				AccelSensor.cpp               \
                        InputEventReader.cpp

LOCAL_SHARED_LIBRARIES := liblog libcutils libdl

include $(BUILD_SHARED_LIBRARY)

endif
