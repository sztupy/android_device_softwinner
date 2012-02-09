#!/system/bin/busybox sh

echo "do preinstall job"
BUSYBOX="/system/bin/busybox"

if [ ! -e /data/system.notfirstrun ]; then		
	$BUSYBOX touch /data/system.notfirstrun	
	/system/bin/sh /system/bin/pm preinstall /system/preinstall
fi

echo "preinstall ok"

