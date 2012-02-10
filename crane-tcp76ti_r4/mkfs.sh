#!/bin/sh

#######################################################################################
#
#	Auto image script for eLDK and android 
#
#######################################################################################

########################################################################################
# we should change this var for different config
ANDROID_ROOT=$ANDROID_BUILD_TOP
#PRODUCT_DIR=$ANDROID_ROOT/out/target/product/aw1618
PRODUCT_DIR=$OUT
LINUXBSP_SRC=$ANDROID_ROOT/../lichee
if [ ! -d $LINUXBSP_SRC ]; then
	echo " cannot find you linux bsp dir (../lichee)!"
	exit	
fi
DRV_DIR=$LINUXBSP_SRC/output/lib/modules/2.6.36-android*
LINUX_IMG=$LINUXBSP_SRC/output

#######################################################################################
# donnot change this var 
PRODUCT_IMAGES=$PRODUCT_DIR/images
PRODUCT_ROOT=$PRODUCT_DIR/root
PRODUCT_SYSTEM=$PRODUCT_DIR/system
PRODUCT_DATA=$PRODUCT_DIR/data
PRODUCT_RECOVERY=$PRODUCT_DIR/recovery/root

ROOTIMG=root.img
SYSTEMIMG=system.img
DATAIMG=data.img
RECOVERYIMG=recovery.img
CACHEIMG=cache.img

ROOTIMG_SIZE=32000
SYSTEMIMG_SIZE=256000
DATAIMG_SIZE=256000
RECOVERYIMG_SIZE=32000
CACHEIMG_SIZE=$SYSTEMIMG_SIZE

mkdir -p $PRODUCT_IMAGES

#######################################################################################
# 

mkimg_flush()
{
	cd $PRODUCT_IMAGES	
	rm -rf *
	
	echo "images flush ok!"
}


mkimg_root()
{	
	echo "****************  start make root.img **************** "	
	
	#copy drv
	if [ -d $PRODUCT_ROOT/drv ]; then
		rm -rf $PRODUCT_ROOT/drv
	fi
	mkdir $PRODUCT_ROOT/drv

	cp $DRV_DIR/* $PRODUCT_ROOT/drv
	# these modules are big and don't fit in the rootfs
	rm $PRODUCT_ROOT/drv/unifi_sdio.ko
	rm $PRODUCT_ROOT/drv/dhd.ko
	rm $PRODUCT_ROOT/drv/hwmw269_dhd.ko
	rm $PRODUCT_ROOT/drv/swbb23_dhd.ko
	rm $PRODUCT_ROOT/drv/usi4329_dhd.ko

	chmod 0755 $PRODUCT_ROOT/drv/*

	cd $PRODUCT_IMAGES		
	mkdir ./root

	echo "alloc disk space..."
	dd if=/dev/zero of=$ROOTIMG bs=1024 count=$ROOTIMG_SIZE
	#mke2fs -F -m 0 -i 2000 $ROOTIMG > /dev/null
	mkfs.ext4 -F $ROOTIMG > /dev/null
	sudo mount -o loop $ROOTIMG ./root

	cd root

	echo "copy from rootfs..."
	cd $PRODUCT_ROOT
	find . \( -path "./dev" -o -path "./acct" -o -path "./cache" -o -path "./config" -o -path "./d" -o -path "./data" -o -path "./mnt" \) -prune -o -print | cpio -o -O $PRODUCT_IMAGES/root.cpio

	cd $PRODUCT_IMAGES/root/
	cpio -id -I $PRODUCT_IMAGES/root.cpio -f *svn*

	cd ..
	sudo umount $PRODUCT_IMAGES/root/
	rm ./root/ -rf
	rm ./root.cpio

	cd $ANDROID_ROOT
	echo "make root.img ok"
}

mkimg_system()
{	
	echo "****************  start make system.img ****************"
	

	cd $PRODUCT_IMAGES
	mkdir ./system

	echo "alloc disk space..."
	dd if=/dev/zero of=$SYSTEMIMG bs=1024 count=${SYSTEMIMG_SIZE}
	#mke2fs -F -m 0 -i 2000 $SYSTEMIMG > /dev/null
	mkfs.ext4 -F $SYSTEMIMG > /dev/null
	sudo mount -o loop $SYSTEMIMG $PRODUCT_IMAGES/system/

	cd system

	echo "copy from systemfs..."

	cd ${PRODUCT_SYSTEM}

	chmod 755 bin/*

	find . \( -path "./dev" -o -path "./acct" -o -path "./cache" -o -path "./config" -o -path "./d" -o -path "./data" -o -path "./mnt" \) -prune -o -print | cpio -o -O ${PRODUCT_IMAGES}/system.cpio
	
	cd ${PRODUCT_IMAGES}/system/
	cpio -id -I ${PRODUCT_IMAGES}/system.cpio -f *svn*

	cd ..
	sleep 1
	sudo umount ${PRODUCT_IMAGES}/system/
	rm ./system/ -rf
	rm ./system.cpio

	cd $ANDROID_ROOT
	echo "make system.img ok"
}
	
mkimg_recovery()
{
	echo "****************  start make recovery.img ****************"

	#copy drv
	if [ -d $PRODUCT_RECOVERY/drv ]; then
		rm -rf $PRODUCT_RECOVERY/drv
	fi
	mkdir $PRODUCT_RECOVERY/drv

	cp $DRV_DIR/hdmi.ko $PRODUCT_RECOVERY/drv/hdmi.ko
	cp $DRV_DIR/disp.ko $PRODUCT_RECOVERY/drv/disp.ko
	cp $DRV_DIR/lcd.ko $PRODUCT_RECOVERY/drv/lcd.ko	
	cp $DRV_DIR/sun4i-keyboard.ko $PRODUCT_RECOVERY/drv/sun4i-keyboard.ko
	chmod 0755 $PRODUCT_RECOVERY/drv/*

	cd $PRODUCT_IMAGES
	mkdir ./recovery

	echo "alloc recovery disk space..."
	dd if=/dev/zero of=$RECOVERYIMG bs=1024 count=${RECOVERYIMG_SIZE}
	mkfs.ext4 -F $RECOVERYIMG > /dev/null
	sudo mount -o loop $RECOVERYIMG ${PRODUCT_IMAGES}/recovery/

	echo "copy from recovery/root..."

	cd $PRODUCT_RECOVERY
	find . -path "./dev" -prune -o -print | cpio -o -O $PRODUCT_IMAGES/recovery.cpio
	
	cd $PRODUCT_IMAGES/recovery/
	cpio -id -I $PRODUCT_IMAGES/recovery.cpio -f *svn*

	cd ..
	sudo umount $PRODUCT_IMAGES/recovery/
	rm ./recovery/ -rf
	rm ./recovery.cpio
	
	echo "make recovery.img ok!"
}

mkimg_flush
mkimg_root
mkimg_system
mkimg_recovery

