LOCAL_PATH:= $(call my-dir)

# JNI library
include $(CLEAR_VARS)
LOCAL_MODULE_TAGS := optional
LOCAL_PRELINK_MODULE	:= false
LOCAL_SHARED_LIBRARIES	+= libdl
LOCAL_STATIC_LIBRARIES	+= 
LOCAL_MODULE_PATH	:= $(TARGET_OUT_SHARED_LIBRARIES)
LOCAL_CFLAGS		+= 
LOCAL_C_INCLUDES	:= $(JNI_H_INCLUDE)

LOCAL_SRC_FILES		:= jni/com_memsic_cal_accel.c

LOCAL_MODULE		:= libaccelcal_jni

include $(BUILD_SHARED_LIBRARY)

# Application package
include $(CLEAR_VARS)

LOCAL_MODULE_TAGS	:= optional
LOCAL_SRC_FILES		:= $(call all-java-files-under, src)
LOCAL_PACKAGE_NAME	:= AccelCalApp

include $(BUILD_PACKAGE)

