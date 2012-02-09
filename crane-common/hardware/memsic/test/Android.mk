LOCAL_PATH := $(call my-dir)


# MEMSIC sensors test cases
# case of read raw data from sensor mmc31xx
include $(CLEAR_VARS)
LOCAL_PRELINK_MODULE	:= false
LOCAL_MODULE_TAGS := eng
LOCAL_SHARED_LIBRARIES	:= 
LOCAL_STATIC_LIBRARIES	:= 
LOCAL_LDLIBS		+= -Idl
LOCAL_CFLAGS		+= -static
LOCAL_C_INCLUDES	+= 

LOCAL_SRC_FILES		:= mmc31xx_test.c

LOCAL_MODULE := mmc31xx_test
include $(BUILD_EXECUTABLE)

# MEMSIC sensors test cases
# case of read raw data from sensor mmc328x
include $(CLEAR_VARS)
LOCAL_PRELINK_MODULE	:= false
LOCAL_MODULE_TAGS := eng
LOCAL_SHARED_LIBRARIES	:= 
LOCAL_STATIC_LIBRARIES	:= 
LOCAL_LDLIBS		+= -Idl
LOCAL_CFLAGS		+= -static
LOCAL_C_INCLUDES	+= 

LOCAL_SRC_FILES		:= mmc328x_test.c

LOCAL_MODULE := mmc328x_test
include $(BUILD_EXECUTABLE)

# MEMSIC sensors test cases
# case of read raw data from sensor mxc6202x
include $(CLEAR_VARS)
LOCAL_PRELINK_MODULE	:= false
LOCAL_MODULE_TAGS := eng
LOCAL_SHARED_LIBRARIES	:= 
LOCAL_STATIC_LIBRARIES	:= 
LOCAL_LDLIBS		+= -Idl
LOCAL_CFLAGS		+= -static
LOCAL_C_INCLUDES	+= 

LOCAL_SRC_FILES		:= mxc6202x_test.c

LOCAL_MODULE := mxc6202x_test
include $(BUILD_EXECUTABLE)

# MEMSIC sensors test cases
# case of read raw data from sensor mxc622x
include $(CLEAR_VARS)
LOCAL_PRELINK_MODULE	:= false
LOCAL_MODULE_TAGS := eng
LOCAL_SHARED_LIBRARIES	:= 
LOCAL_STATIC_LIBRARIES	:= 
LOCAL_LDLIBS		+= -Idl
LOCAL_CFLAGS		+= -static
LOCAL_C_INCLUDES	+= 

LOCAL_SRC_FILES		:= mxc622x_test.c

LOCAL_MODULE := mxc622x_test
include $(BUILD_EXECUTABLE)

# MEMSIC sensors test cases
# case of test ecompass
include $(CLEAR_VARS)
LOCAL_PRELINK_MODULE	:= false
LOCAL_MODULE_TAGS := eng
LOCAL_SHARED_LIBRARIES	:= 
LOCAL_STATIC_LIBRARIES	:= 
LOCAL_LDLIBS		+= -Idl
LOCAL_CFLAGS		+= -static
LOCAL_C_INCLUDES	+= 

LOCAL_SRC_FILES		:= ecs_data_test.c

LOCAL_MODULE := mecs_data_test
include $(BUILD_EXECUTABLE)

# MEMSIC sensors test cases
# case of test ecompass
include $(CLEAR_VARS)
LOCAL_PRELINK_MODULE	:= false
LOCAL_MODULE_TAGS := eng
LOCAL_SHARED_LIBRARIES	:= 
LOCAL_STATIC_LIBRARIES	:= 
LOCAL_LDLIBS		+= -Idl
LOCAL_CFLAGS		+= -static
LOCAL_C_INCLUDES	+= 

LOCAL_SRC_FILES		:= ecs_ctrl_test.c

LOCAL_MODULE := mecs_ctrl_test
include $(BUILD_EXECUTABLE)

