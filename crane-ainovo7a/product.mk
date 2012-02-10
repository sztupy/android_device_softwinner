# crane evb product config
$(call inherit-product, device/softwinner/crane-common/ProductCommon.mk)

#add overlay
DEVICE_PACKAGE_OVERLAYS := \
	device/softwinner/crane-ainovo7a/overlay

PRODUCT_COPY_FILES += \
	device/softwinner/crane-ainovo7a/ueventd.sun4i.rc:root/ueventd.sun4i.rc \
	device/softwinner/crane-ainovo7a/init.sun4i.rc:root/init.sun4i.rc \
	device/softwinner/crane-ainovo7a/sun4i-keyboard.kl:system/usr/keylayout/sun4i-keyboard.kl \
	device/softwinner/crane-ainovo7a/sun4i-ir.kl:system/usr/keylayout/sun4i-ir.kl \
	device/softwinner/crane-ainovo7a/hv_keypad.kl:system/usr/keylayout/hv_keypad.kl \
	device/softwinner/crane-ainovo7a/media_profiles.xml:system/etc/media_profiles.xml \
	device/softwinner/crane-ainovo7a/camera.cfg:system/etc/camera.cfg

# fake kernel, fstab for recovery
PRODUCT_COPY_FILES += \
	device/softwinner/crane-ainovo7a/kernel:kernel \
	device/softwinner/crane-ainovo7a/recovery.fstab:recovery.fstab

# vold.fstab
PRODUCT_COPY_FILES += \
	device/softwinner/crane-ainovo7a/vold.fstab:system/etc/vold.fstab

PRODUCT_PROPERTY_OVERRIDES += \
	ro.additionalmounts=/mnt/extern_sd;/mnt/usbhost1 \
	ro.additionalshares=/mnt/extern_sd;/mnt/usbhost1 \
	ro.udisk.lable=crane-ainovo7a \
	ro.product.firmware=2.0.3
	
# Overrides
PRODUCT_BRAND  := softwinners
PRODUCT_NAME   := crane_Ainol_Novo7A
PRODUCT_DEVICE := crane-ainovo7a
PRODUCT_MODEL := Ainol Novo7 Advanced

