# BoardConfig.mk
#
# Product-specific compile-time definitions.
#

TARGET_NO_BOOTLOADER := true
TARGET_PROVIDES_INIT_RC :=true

# recovery stuff
TARGET_NO_KERNEL := false
INSTALLED_KERNEL_TARGET := kernel
#TARGET_RECOVERY_UI_LIB := crane_recovery
#TARGET_RELEASETOOLS_EXTENSIONS := 

TARGET_USERIMAGES_USE_EXT4 := true
BOARD_FLASH_BLOCK_SIZE := 4096
BOARD_SYSTEMIMAGE_PARTITION_SIZE := 536870912
#BOARD_USERDATAIMAGE_PARTITION_SIZE := 1073741824

# cpu stuff
TARGET_CPU_ABI := armeabi-v7a
TARGET_CPU_ABI2 := armeabi

# Enable NEON feature
TARGET_ARCH_VARIANT := armv7-a-neon
ARCH_ARM_HAVE_TLS_REGISTER := true 

TARGET_BOARD_PLATFORM := exDroid
TARGET_BOOTLOADER_BOARD_NAME := crane

# audio & camera & cedarx
CEDARX_CHIP_VERSION := F23

HAVE_HTC_AUDIO_DRIVER := false
BOARD_USES_GENERIC_AUDIO := false 
BOARD_USES_ALSA_AUDIO := true

#USE_CAMERA_STUB := false
USE_CAMERA_STUB := true
ifeq ($(USE_CAMERA_STUB), false)
BOARD_CAMERA_LIBRARIES := libcamera
endif

# gsensor config var
# we can now use variable :  mxc622x mma7660 mma8451
#BOARD_USES_GSENSOR_TYPE := mxc622x

# gpu
BOARD_USE_GPU := true

# wifi 
AVE_CUSTOM_WIFI_DRIVER_2 := true
BOARD_WPA_SUPPLICANT_DRIVER := WEXT
WPA_SUPPLICANT_VERSION := VER_0_6_X
# usb wifi "rtl8192cu"; sdio wifi "nanowifi"
BOARD_USR_WIFI := nanowifi

#gps 
#"simulator":taget board does not have a gps hardware module;"haiweixun":use the gps module offer by haiweixun 
BOARD_USES_GPS_TYPE := simulator

# use our own libhardware_legacy
BOARD_USES_PRIV_HARDWARE_LEGACY := true

# use our own su for root
BOARD_USES_ROOT_SU := true

# hardware module include file path
TARGET_HARDWARE_INCLUDE := $(TOP)/device/softwinner/crane-common/hardware/include


