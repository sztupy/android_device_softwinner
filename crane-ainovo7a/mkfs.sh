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

mkimg_beautify()
{
	echo "Would you want beautify it ? (y/n)"
	read input
	
	if [ $input = "y" -o $input = "Y" ]
	then
	#backup
	mkdir -p $PRODUCT_DIR/beautify_backup/app/
	cp $PRODUCT_SYSTEM/app/SystemUI.apk $PRODUCT_DIR/beautify_backup/app/
	cp $PRODUCT_SYSTEM/app/Launcher2.apk $PRODUCT_DIR/beautify_backup/app/
	mkdir -p $PRODUCT_DIR/beautify_backup/framework/
	cp $PRODUCT_SYSTEM/framework/framework-res.apk $PRODUCT_DIR/beautify_backup/framework/
	mkdir -p $PRODUCT_DIR/beautify_backup/fonts/
	cp $PRODUCT_SYSTEM/fonts/DroidSans.ttf $PRODUCT_DIR/beautify_backup/fonts/
	cp $PRODUCT_SYSTEM/fonts/DroidSans-Bold.ttf $PRODUCT_DIR/beautify_backup/fonts/
	cp $PRODUCT_SYSTEM/fonts/DroidSansFallback.ttf $PRODUCT_DIR/beautify_backup/fonts/
	#beautify
	cp $DEVICE/apk/*.apk $PRODUCT_SYSTEM/app/
	cp $DEVICE/framework/framework-res.apk $PRODUCT_SYSTEM/framework/
	cp $DEVICE/fonts/*.ttf $PRODUCT_SYSTEM/fonts/
	rm $PRODUCT_SYSTEM/app/Launcher2.apk
	
	elif [ -d $PRODUCT_DIR/beautify_backup ]
	then
	rm $PRODUCT_SYSTEM/app/ADWLauncherEX.apk
	rm $PRODUCT_SYSTEM/app/AppIcons.apk
	rm $PRODUCT_SYSTEM/app/DeskClock.apk
	rm $PRODUCT_SYSTEM/app/WallPapers.apk
	rm $PRODUCT_SYSTEM/app/NotificationToggle.apk
	cp $PRODUCT_DIR/beautify_backup/app/*.apk $PRODUCT_SYSTEM/app/
	cp $PRODUCT_DIR/beautify_backup/fonts/*.ttf $PRODUCT_SYSTEM/fonts/
	cp $PRODUCT_DIR/beautify_backup/framework/framework-res.apk $PRODUCT_SYSTEM/framework/
	
	fi
	
	cp $DEVICE/apk/ES_fileexplorer.apk $PRODUCT_SYSTEM/app/
	cp $DEVICE/apk/ESpm.apk $PRODUCT_SYSTEM/app/
	rm $PRODUCT_SYSTEM/app/QuickSearchBox.apk
	rm $PRODUCT_SYSTEM/app/SpeechRecorder.apk
	rm $PRODUCT_SYSTEM/app/SpareParts.apk
	rm $PRODUCT_SYSTEM/app/Development.apk
}
###########################

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

	#cp -r $DRV_DIR/* $PRODUCT_ROOT/drv
	cp $DRV_DIR/ump.ko $PRODUCT_ROOT/drv	
	cp $DRV_DIR/mali.ko $PRODUCT_ROOT/drv	
	cp $DRV_DIR/nano_if.ko $PRODUCT_ROOT/drv
	cp $DRV_DIR/nano_ksdio.ko $PRODUCT_ROOT/drv
	cp $DRV_DIR/x_mac.axf $PRODUCT_ROOT/drv/x_mac.axf 
	cp $DRV_DIR/videobuf-core.ko $PRODUCT_ROOT/drv/videobuf-core.ko
	cp $DRV_DIR/videobuf-dma-contig.ko $PRODUCT_ROOT/drv/videobuf-dma-contig.ko
cp $DRV_DIR/gc0308.ko $PRODUCT_ROOT/drv/gc0308.ko
cp $DRV_DIR/gt2005.ko $PRODUCT_ROOT/drv/gt2005.ko
cp $DRV_DIR/sun4i_csi0.ko $PRODUCT_ROOT/drv/sun4i_csi0.ko
cp $DRV_DIR/sun4i_csi1.ko $PRODUCT_ROOT/drv/sun4i_csi1.ko	
	cp $DRV_DIR/ov7670.ko $PRODUCT_ROOT/drv/ov7670.ko
	cp $DRV_DIR/sun4i_csi0.ko $PRODUCT_ROOT/drv/sun4i_csi0.ko
	#cp $DRV_DIR/sun4i-ts.ko $PRODUCT_ROOT/drv/sun4i-ts.ko
	cp $DRV_DIR/hv2605.ko	$PRODUCT_ROOT/drv/hv2605.ko
	cp $DRV_DIR/ft5x_ts.ko $PRODUCT_ROOT/drv/ft5x_ts.ko
	cp $DRV_DIR/sun4i-keyboard.ko $PRODUCT_ROOT/drv/sun4i-keyboard.ko
	cp $DRV_DIR/8192cu.ko $PRODUCT_ROOT/drv/8192cu.ko
	cp $DRV_DIR/bma250.ko $PRODUCT_ROOT/drv/bma250.ko
	cp $DRV_DIR/hdmi.ko $PRODUCT_ROOT/drv/hdmi.ko
	cp $DRV_DIR/disp.ko $PRODUCT_ROOT/drv/disp.ko
	cp $DRV_DIR/lcd.ko $PRODUCT_ROOT/drv/lcd.ko	
	cp $DRV_DIR/usbnet.ko $PRODUCT_ROOT/drv/usbnet.ko
	cp $DRV_DIR/asix.ko $PRODUCT_ROOT/drv/asix.ko	
	cp $DRV_DIR/goodix_touch.ko $PRODUCT_ROOT/drv/goodix_touch.ko
	cp $DRV_DIR/sun4i-vibrator.ko $PRODUCT_ROOT/drv/sun4i-vibrator.ko
	cp $DRV_DIR/rtl8150.ko $PRODUCT_ROOT/drv/rtl8150.ko	
	cp $DRV_DIR/qf9700.ko $PRODUCT_ROOT/drv/qf9700.ko	
	cp $DRV_DIR/mcs7830.ko $PRODUCT_ROOT/drv/mcs7830.ko	
	cp $DRV_DIR/sun4i-ir.ko $PRODUCT_ROOT/drv/sun4i-ir.ko

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
	find . \( -path "./dev" -o -path "./acct" -o -path "./cache" -o -path "./config" -o -path "./d" -o -path "./data" -o -path "./mnt" \) -prune -o -print | cpio -o -O ${PRODUCT_IMAGES}/system.cpio
	
	cd ${PRODUCT_IMAGES}/system/
	cpio -id -I ${PRODUCT_IMAGES}/system.cpio -f *svn*

	cd ..
	sudo umount ${PRODUCT_IMAGES}/system/
	rm ./system/ -rf
	rm ./system.cpio

	cd $ANDROID_ROOT
	echo "make system.img ok"
}

mkimg_data()
{		
	echo "****************  start make data.img ****************"
	
	cd $PRODUCT_IMAGES
	mkdir ./data

	echo "alloc datafs disk space..."
	dd if=/dev/zero of=$DATAIMG bs=1024 count=${DATAIMG_SIZE}
	mke2fs -F -m 0 -i 2000 $DATAIMG > /dev/null
	sudo mount -o loop $DATAIMG ${PRODUCT_IMAGES}/data/

	echo "copy from dataFS..."

	cd $PRODUCT_DATA
	find . -path "./dev" -prune -o -print | cpio -o -O $PRODUCT_IMAGES/data.cpio
	
	cd $PRODUCT_IMAGES/data/
	cpio -id -I $PRODUCT_IMAGES/data.cpio -f *svn*

	cd ..
	sudo umount $PRODUCT_IMAGES/data/
	rm ./data/ -rf
	rm ./data.cpio
	
	cd $ANDROID_ROOT

	echo "make data.img ok"
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

	mkimg_beautify
	mkimg_flush
	mkimg_root
	mkimg_system
	#mkimg_data
	mkimg_recovery
	#cp $LINUX_IMG/bImage $PRODUCT_IMAGES/bImage

	# linux pack, not verified!
#	if [ -e $ANDROID_ROOT/../tools/crane-evb-linux/wboot/image.sh ]; then
#		pushd .
#		cd $ANDROID_ROOT/../tools/crane-evb-linux/wboot
#		sh image.sh
#		popd
#	else
#		echo "cannot found tools to generate ePDKV100.img"
#	fi



