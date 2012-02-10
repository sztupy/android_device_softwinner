# full crane product config
#$(call inherit-product, $(SRC_TARGET_DIR)/product/generic.mk)
$(call inherit-product, device/softwinner/crane-common/device_base.mk)
$(call inherit-product, $(SRC_TARGET_DIR)/product/languages_full.mk)

DEVICE_PACKAGE_OVERLAYS := device/softwinner/crane-common/overlay

PRODUCT_PACKAGES += \
	copybit.sun4i \
    lights.sun4i \
	overlay.sun4i \
	display.sun4i \
	gralloc.sun4i \
	gps.sun4i \
	libhardware_legacy \
	chat	

# Filesystem management tools
PRODUCT_PACKAGES += \
	make_ext4fs

# init.rc
PRODUCT_COPY_FILES += \
	device/softwinner/crane-common/init.rc:root/init.rc

# wifi conf
PRODUCT_COPY_FILES += \
	device/softwinner/crane-common/wpa_supplicant.conf:system/etc/wifi/wpa_supplicant.conf \
	device/softwinner/crane-common/dhcpcd.conf:system/etc/dhcpcd/dhcpcd.conf \
	device/softwinner/crane-common/gps.conf:system/etc/gps.conf 

# mali lib so
PRODUCT_COPY_FILES += \
	device/softwinner/crane-common/egl/gralloc.sun4i.so:system/lib/hw/gralloc.sun4i.so \
	device/softwinner/crane-common/egl/libMali.so:system/lib/libMali.so \
	device/softwinner/crane-common/egl/libUMP.so:system/lib/libUMP.so \
	device/softwinner/crane-common/egl/egl.cfg:system/lib/egl/egl.cfg \
	device/softwinner/crane-common/egl/libEGL_mali.so:system/lib/egl/libEGL_mali.so \
	device/softwinner/crane-common/egl/libGLESv1_CM_mali.so:system/lib/egl/libGLESv1_CM_mali.so \
	device/softwinner/crane-common/egl/libGLESv2_mali.so:system/lib/egl/libGLESv2_mali.so

# bin tools
PRODUCT_COPY_FILES += \
	device/softwinner/crane-common/bin/fsck.exfat:system/bin/fsck.exfat \
	device/softwinner/crane-common/bin/mkfs.exfat:system/bin/mkfs.exfat \
	device/softwinner/crane-common/bin/mount.exfat:system/bin/mount.exfat \
	device/softwinner/crane-common/bin/ntfs-3g:system/bin/ntfs-3g \
	device/softwinner/crane-common/bin/ntfs-3g.probe:system/bin/ntfs-3g.probe \
	device/softwinner/crane-common/bin/mkntfs:system/bin/mkntfs \
	device/softwinner/crane-common/bin/busybox:system/bin/busybox \
	device/softwinner/crane-common/bin/e2fsck:system/bin/e2fsck \
	device/softwinner/crane-common/bin/usb_modeswitch:system/bin/usb_modeswitch \
	device/softwinner/crane-common/bin/usb_modeswitch.sh:system/etc/usb_modeswitch.sh

PRODUCT_COPY_FILES += \
	device/softwinner/crane-common/preinstall.sh:/system/bin/preinstall.sh 
	
PRODUCT_COPY_FILES += \
	device/softwinner/crane-common/rild/ip-down:/system/etc/ppp/ip-down \
	device/softwinner/crane-common/rild/ip-up:/system/etc/ppp/ip-up \
	device/softwinner/crane-common/rild/call-pppd:/system/etc/ppp/call-pppd \
	device/softwinner/crane-common/rild/liballwinner-ril.so:/system/lib/liballwinner-ril.so

#premission feature
PRODUCT_COPY_FILES += \
    frameworks/base/data/etc/android.hardware.location.xml:system/etc/permissions/android.hardware.location.xml \
    frameworks/base/data/etc/android.hardware.location.gps.xml:system/etc/permissions/android.hardware.location.gps.xml \
    frameworks/base/data/etc/android.hardware.sensor.accelerometer.xml:system/etc/permissions/android.hardware.sensor.accelerometer.xml \
    frameworks/base/data/etc/android.hardware.sensor.compass.xml:system/etc/permissions/android.hardware.sensor.compass.xml \
    frameworks/base/data/etc/android.hardware.wifi.xml:system/etc/permissions/android.hardware.wifi.xml \
    frameworks/base/data/etc/android.hardware.touchscreen.xml:system/etc/permissions/android.hardware.touchscreen.xml \
    frameworks/base/data/etc/android.hardware.touchscreen.multitouch.distinct.xml:system/etc/permissions/android.hardware.touchscreen.multitouch.distinct.xml \
    
#
PRODUCT_PROPERTY_OVERRIDES += \
	dalvik.vm.heapsize=32m \
	debug.egl.hw=1 \
	ro.opengles.version=131072 \
	ro.ril.wake_lock_timeout=1000 \
	ro.kernel.android.checkjni=0 \
	persist.sys.timezone=Europe/London \
	persist.sys.language=en \
	persist.sys.country=UK \
	keyguard.no_require_sim=true \
	ro.com.android.dataroaming=true \
	ro.ril.hsxpa=1 \
	ro.ril.gprsclass=10 \
	wifi.supplicant_scan_interval=15 	

# Overrides
PRODUCT_BRAND  := softwinners
PRODUCT_NAME   := full_crane
PRODUCT_DEVICE := crane


