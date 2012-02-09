# BoardConfig.mk
#
# Product-specific compile-time definitions.
#

include device/softwinner/crane-common/BoardConfigCommon.mk

#BOARD_USR_WIFI := rtl8192cu

# Gsensor board config
# we can use this string : mma7660, mxc622x , bma250
BOARD_USES_GSENSOR_TYPE := bma250
BOARD_GSENSOR_DIRECT_X := true 
BOARD_GSENSOR_DIRECT_Y := true
BOARD_GSENSOR_DIRECT_Z := true

BOARD_GSENSOR_XY_REVERT := false 

