# crane evb product config
$(call inherit-product, device/softwinner/crane-common/ProductCommon.mk)

#add overlay
DEVICE_PACKAGE_OVERLAYS := \
	device/softwinner/crane-tcp76ti_r4/overlay

PRODUCT_COPY_FILES += \
	device/softwinner/crane-tcp76ti_r4/ueventd.sun4i.rc:root/ueventd.sun4i.rc \
	device/softwinner/crane-tcp76ti_r4/init.sun4i.rc:root/init.sun4i.rc \
	device/softwinner/crane-tcp76ti_r4/sun4i-keyboard.kl:system/usr/keylayout/sun4i-keyboard.kl \
	device/softwinner/crane-tcp76ti_r4/sun4i-ir.kl:system/usr/keylayout/sun4i-ir.kl \
	device/softwinner/crane-tcp76ti_r4/hv_keypad.kl:system/usr/keylayout/hv_keypad.kl \
	device/softwinner/crane-tcp76ti_r4/media_profiles.xml:system/etc/media_profiles.xml \
	device/softwinner/crane-tcp76ti_r4/camera.cfg:system/etc/camera.cfg

# fake kernel, fstab for recovery
PRODUCT_COPY_FILES += \
	device/softwinner/crane-tcp76ti_r4/kernel:kernel \
	device/softwinner/crane-tcp76ti_r4/recovery.fstab:recovery.fstab

# vold.fstab
PRODUCT_COPY_FILES += \
	device/softwinner/crane-tcp76ti_r4/vold.fstab:system/etc/vold.fstab

# init log
PRODUCT_COPY_FILES += \
	device/softwinner/crane-tcp76ti_r4/media/bootanimation.zip:system/media/bootanimation.zip \

PRODUCT_PROPERTY_OVERRIDES += \
	ro.additionalmounts=/mnt/extern_sd;/mnt/usbhost1 \
	ro.additionalshares=/mnt/extern_sd;/mnt/usbhost1 \
	ro.udisk.lable=crane-tcp76ti_r4 \
	ro.product.firmware=2.0.3
	
# Overrides
PRODUCT_BRAND  := softwinners
PRODUCT_NAME   := crane_Teclast_P76TI_REV4
PRODUCT_DEVICE := crane-tcp76ti_r4
PRODUCT_MODEL := Teclast P76TI REV4

