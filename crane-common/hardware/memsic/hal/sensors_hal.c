/*****************************************************************************
 *  Copyright Statement:
 *  --------------------
 *  This software is protected by Copyright and the information and source code
 *  contained herein is confidential. The software including the source code
 *  may not be copied and the information contained herein may not be used or
 *  disclosed except with the written permission of MEMSIC Inc. (C) 2010
 *****************************************************************************/

/**
 * @file
 * @author  Robbie Cao<hjcao@memsic.cn>
 *
 * @brief
 * This file implement sensor hardware abstract layer for Android. <br>
 * It’s an implementation of the APIs defined in ‘sensors.h’ under 
 * Android source tree.
 */

#define LOG_TAG "SensorHAL"

#include <memory.h>
#include <sys/time.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <dirent.h>
#include <math.h>
#include <poll.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <errno.h>

#include <linux/input.h>

#include <hardware/hardware.h>
#include <hardware/sensors.h>

#include <cutils/atomic.h>
#include <utils/Log.h>


#define SENSORS_HAL_DEBUG		1

#define SENSORS_SUPPORT_COUNT		3

#define SUPPORTED_SENSORS		((1 << SENSORS_SUPPORT_COUNT) - 1)

#define ID_A				(0)
#define ID_M 				(1)
#define ID_O 				(2)
#define ID_T				(3)
#define ID_L				(4)
#define ID_P				(5)
#define ID_GY				(6)

#define SENSORS_ACCELERATION		(1 << ID_A)
#define SENSORS_MAGNETIC_FIELD		(1 << ID_M)
#define SENSORS_ORIENTATION		(1 << ID_O)
#define SENSORS_TEMPERATURE		(1 << ID_T)
#define SENSORS_LIGHT			(1 << ID_L)
#define SENSORS_PROXIMITY		(1 << ID_P)
#define SENSORS_GYROSCOPE		(1 << ID_GY)

#define ECS_CTRL_DEV_NAME		"/dev/ecompass_ctrl"


#define EVENT_TYPE_ACCEL_X		ABS_X
#define EVENT_TYPE_ACCEL_Y		ABS_Y
#define EVENT_TYPE_ACCEL_Z		ABS_Z
#define EVENT_TYPE_ACCEL_STATUS		ABS_WHEEL

#define EVENT_TYPE_MAGV_X		ABS_HAT0X
#define EVENT_TYPE_MAGV_Y		ABS_HAT0Y
#define EVENT_TYPE_MAGV_Z		ABS_BRAKE
#define EVENT_TYPE_MAGV_STATUS		ABS_GAS

#define EVENT_TYPE_ORIENT_YAW		ABS_RX
#define EVENT_TYPE_ORIENT_PITCH		ABS_RY
#define EVENT_TYPE_ORIENT_ROLL		ABS_RZ
#define EVENT_TYPE_ORIENT_STATUS	ABS_RUDDER

// conversion of acceleration data to SI units (m/s^2)
// 32768 = 1G
#define CONVERT_A			(GRAVITY_EARTH / 32768.0f)

#ifdef GSENSOR_DIRECT_X
#define CONVERT_A_X                 (CONVERT_A)
#else
#define CONVERT_A_X                 (-CONVERT_A)
#endif

#ifdef GSENSOR_DIRECT_Y
#define CONVERT_A_Y                 (CONVERT_A)
#else
#define CONVERT_A_Y                 (-CONVERT_A)
#endif 

#ifdef GSENSOR_DIRECT_Z
#define CONVERT_A_Z                 (CONVERT_A)
#else
#define CONVERT_A_Z                 (-CONVERT_A)
#endif



// conversion of magnetic data to uT units
// 32768 = 1Guass
#define CONVERT_M			(100.f / 32768.0f)
#define CONVERT_M_X			(CONVERT_M)
#define CONVERT_M_Y			(CONVERT_M)
#define CONVERT_M_Z			(CONVERT_M)

// 65536 = 360Degree
#define CONVERT_O			(360.0f / 65536.0f)
#define CONVERT_O_Y			(CONVERT_O)
#define CONVERT_O_P			(CONVERT_O)
#define CONVERT_O_R			(CONVERT_O)

/* Use 'e' as magic number */
#define ECOMPASS_IOM			'e'

/* IOCTLs for ECOMPASS device */
#define ECOMPASS_IOC_SET_MODE		_IOW(ECOMPASS_IOM, 0x00, short)
#define ECOMPASS_IOC_SET_DELAY		_IOW(ECOMPASS_IOM, 0x01, short)
#define ECOMPASS_IOC_GET_DELAY		_IOR(ECOMPASS_IOM, 0x02, short)

#define ECOMPASS_IOC_SET_AFLAG		_IOW(ECOMPASS_IOM, 0x10, short)
#define ECOMPASS_IOC_GET_AFLAG		_IOR(ECOMPASS_IOM, 0x11, short)
#define ECOMPASS_IOC_SET_MFLAG		_IOW(ECOMPASS_IOM, 0x12, short)
#define ECOMPASS_IOC_GET_MFLAG		_IOR(ECOMPASS_IOM, 0x13, short)
#define ECOMPASS_IOC_SET_OFLAG		_IOW(ECOMPASS_IOM, 0x14, short)
#define ECOMPASS_IOC_GET_OFLAG		_IOR(ECOMPASS_IOM, 0x15, short)

#define ECOMPASS_IOC_SET_APARMS		_IOW(ECOMPASS_IOM, 0x20, int[4])
#define ECOMPASS_IOC_GET_APARMS		_IOR(ECOMPASS_IOM, 0x21, int[4])
#define ECOMPASS_IOC_SET_MPARMS		_IOW(ECOMPASS_IOM, 0x22, int[4])
#define ECOMPASS_IOC_GET_MPARMS		_IOR(ECOMPASS_IOM, 0x23, int[4])
#define ECOMPASS_IOC_SET_OPARMS_YAW	_IOW(ECOMPASS_IOM, 0x24, int[4])
#define ECOMPASS_IOC_GET_OPARMS_YAW	_IOR(ECOMPASS_IOM, 0x25, int[4])
#define ECOMPASS_IOC_SET_OPARMS_PITCH	_IOW(ECOMPASS_IOM, 0x26, int[4])
#define ECOMPASS_IOC_GET_OPARMS_PITCH	_IOR(ECOMPASS_IOM, 0x27, int[4])
#define ECOMPASS_IOC_SET_OPARMS_ROLL	_IOW(ECOMPASS_IOM, 0x28, int[4])
#define ECOMPASS_IOC_GET_OPARMS_ROLL	_IOR(ECOMPASS_IOM, 0x29, int[4])

#define ECOMPASS_IOC_SET_YPR		_IOW(ECOMPASS_IOM, 0x30, int[12])


/**
 * @brief
 * Sensor poll device context
 */
struct sensors_poll_context_t {
	struct sensors_poll_device_t	device;
	int				ecs_fd;
	int				events_fd;
	uint32_t			active_sensors;
	uint32_t			pending_sensors;
	sensors_event_t			sensors[SENSORS_SUPPORT_COUNT];
};

static int sSensorAccr[SENSORS_SUPPORT_COUNT] = {0, 0, 0};	// default: SENSOR_STATUS_UNRELIABLE

static struct sensor_t sensors_list[SENSORS_SUPPORT_COUNT] = {
	// accelerometer
	{
	#if ((defined DEVICE_ACC_MXC6202XG) || (defined DEVICE_ACC_MXC6202XM))
		name: "MXC6202X",
		vendor: "MEMSIC",
		version: 0,
		handle: SENSORS_HANDLE_BASE + ID_A,
		type: SENSOR_TYPE_ACCELEROMETER,
		maxRange: 2.0,
		resolution: GRAVITY_EARTH/512.0f,	// 9.8 / 512 (m/s2)
		power: 0.005,		// 5mA
		minDelay: 0,
		reserved: {0}
	#elif ((defined DEVICE_ACC_MXC6202XH) || (defined DEVICE_ACC_MXC6202XN))
		name: "MXC6202X",
		vendor: "MEMSIC",
		version: 0,
		handle: SENSORS_HANDLE_BASE + ID_A,
		type: SENSOR_TYPE_ACCELEROMETER,
		maxRange: 2.0,
		resolution: GRAVITY_EARTH/128.0f,	// 9.8 / 128 (m/s2)
		power: 0.005,		// 5mA
		minDelay: 0,
		reserved: {0}
	#elif (defined DEVICE_ACC_MXC622X)
		name: "MXC622X",
		vendor: "MEMSIC",
		version: 0,
		handle: SENSORS_HANDLE_BASE + ID_A,
		type: SENSOR_TYPE_ACCELEROMETER,
		maxRange: 2.0,
		resolution: GRAVITY_EARTH/64.0f,	// 9.8 / 128 (m/s2)
		power: 0.005,		// 5mA
		minDelay: 0,
		reserved: {0}
	#elif (defined DEVICE_ACC_BMA220)
		name: "BMA220",
		vendor: "BOSCH",
		version: 0,
		handle: SENSORS_HANDLE_BASE + ID_A,
		type: SENSOR_TYPE_ACCELEROMETER,
		maxRange: 2.0,
		resolution: 0.6,	// 9.8 / 16 (m/s2)
		power: 0.005,		// 5mA?
		minDelay: 0,
		reserved: {0}
	#elif (defined DEVICE_ACC_BMA023)
		name: "BMA023",
		vendor: "BOSCH",
		version: 0,
		handle: SENSORS_HANDLE_BASE + ID_A,
		type: SENSOR_TYPE_ACCELEROMETER,
		maxRange: 2.0,
		resolution: 0.038,	// 9.8 / 256 (m/s2)
		power: 0.005,		// 5mA?
		minDelay: 0,
		reserved: {0}
	#elif (defined DEVICE_ACC_KR3DM)
		name: "KR3DM",
		vendor: "ST",
		version: 0,
		handle: SENSORS_HANDLE_BASE + ID_A,
		type: SENSOR_TYPE_ACCELEROMETER,
		maxRange: 2.0,
		resolution: 0.15,	// 9.8 / 64 (m/s2)
		power: 0.005,		// 5mA?
		minDelay: 0,
		reserved: {0}
	#else
		#error "Acc sensor device not specify!"
	#endif
	},
	// magnetic field meter
	{
	#if (defined DEVICE_MAG_MMC312X)
		name: "MMC312X",
		vendor: "MEMSIC",
		version: 0,
		handle: SENSORS_HANDLE_BASE + ID_M,
		type: SENSOR_TYPE_MAGNETIC_FIELD,
		maxRange: 2.0,
		resolution: 100.0f/512.0f,	// 100 / 512 (uT)
		power: 0.003,			// 3mA
		minDelay: 0,
		reserved: {0}
	#elif (defined DEVICE_MAG_MMC314X)
		name: "MMC314X",
		vendor: "MEMSIC",
		version: 0,
		handle: SENSORS_HANDLE_BASE + ID_M,
		type: SENSOR_TYPE_MAGNETIC_FIELD,
		maxRange: 4.0,
		resolution: 100.0f/512.0f,	// 100 / 512 (uT)
		power: 0.003,			// 3mA
		minDelay: 0,
		reserved: {0}
	#elif (defined DEVICE_MAG_MMC328X)
		name: "MMC328X",
		vendor: "MEMSIC",
		version: 0,
		handle: SENSORS_HANDLE_BASE + ID_M,
		type: SENSOR_TYPE_MAGNETIC_FIELD,
		maxRange: 4.0,
		resolution: 100.0f/512.0f,	// 100 / 512 (uT)
		power: 0.003,			// 3mA
		minDelay: 0,
		reserved: {0}
	#else
		#error "Mag sensor device not specify!"
	#endif
	},
	// orientation
	{
		name: "Mag & Acc Combo Orientation Sensor",
		vendor: "MEMSIC",
		version: 0,
		handle: SENSORS_HANDLE_BASE + ID_O,
		type: SENSOR_TYPE_ORIENTATION,
		maxRange: 2.0,
		resolution: 1.0,	// FIXME: ?
		power: 0.0,		// FIXME: ?
		minDelay: 0,
		reserved: {0}
	}
};

static int control_open_input(int mode)
{
	/* scan all input drivers and look for "ecompass_data" */
	int fd = -1;
	const char *dirname = "/dev/input";
	char devname[PATH_MAX];
	char *filename;
	DIR *dir;
	struct dirent *de;

	dir = opendir(dirname);
	if (dir == NULL) {
		return -1;
	}

	strcpy(devname, dirname);
	filename = devname + strlen(devname);
	*filename++ = '/';
	while ((de = readdir(dir))) {
		if ((de->d_name[0] == '.' && de->d_name[1] == '\0') || 
		    (de->d_name[0] == '.' && de->d_name[1] == '.'  && 
		     de->d_name[2] == '\0')) {
			/* ignore .(current) and ..(top) directory */
			continue;
		}
		strcpy(filename, de->d_name);
		fd = open(devname, mode);
		if (fd >= 0) {
			char name[80];
			if (ioctl(fd, EVIOCGNAME(sizeof(name) - 1), &name) < 1) {
				name[0] = '\0';
			}
			if (!strcmp(name, "ecompass_data")) {
				LOGD("Using %s (name=%s)", devname, name);
				break;
			}
			close(fd);
			fd = -1;
		}
	}
	closedir(dir);

	if (fd < 0) {
		LOGD("Couldn't find or open 'compass' driver (%s)", strerror(errno));
	}

	return fd;
}

static int control_open_ecs(struct sensors_poll_context_t* dev)
{
	if (dev->ecs_fd < 0) {
		dev->ecs_fd = open(ECS_CTRL_DEV_NAME, O_RDONLY);
		LOGE_IF(dev->ecs_fd < 0, "Couldn't open %s (%s)",
			ECS_CTRL_DEV_NAME, strerror(errno));
		if (dev->ecs_fd >= 0) {
			dev->active_sensors = 0;
		}
	}

	return dev->ecs_fd;
}

static void control_close_ecs(struct sensors_poll_context_t* dev)
{
	if (dev->ecs_fd >= 0) {
		close(dev->ecs_fd);
		dev->ecs_fd = -1;
	}
}

static void control_enable_disable_sensors(int fd, uint32_t sensors, uint32_t mask)
{
	if (fd < 0) {
		return;
	}
	short flags;

	if (mask & SENSORS_ACCELERATION) {
		flags = (sensors & SENSORS_ACCELERATION) ? 1 : 0;
		if (ioctl(fd, ECOMPASS_IOC_SET_AFLAG, &flags) < 0) {
			LOGE("ECOMPASS_IOC_SET_AFLAG error (%s)", 
				strerror(errno));
		}
	}
	if (mask & SENSORS_MAGNETIC_FIELD) {
		flags = (sensors & SENSORS_MAGNETIC_FIELD) ? 1 : 0;
		if (ioctl(fd, ECOMPASS_IOC_SET_MFLAG, &flags) < 0) {
			LOGE("ECOMPASS_IOC_SET_MFLAG error (%s)", 
				strerror(errno));
		}
	}
	if (mask & SENSORS_ORIENTATION) {
		flags = (sensors & SENSORS_ORIENTATION) ? 1 : 0;
		if (ioctl(fd, ECOMPASS_IOC_SET_OFLAG, &flags) < 0) {
			LOGE("ECOMPASS_IOC_SET_OFLAG error (%s)", 
				strerror(errno));
		}
	}
}

static uint32_t control_read_sensors_state(int fd)
{
	if (fd < 0) {
		return 0;
	}

	short flags;
	uint32_t sensors = 0;

	// read the actual value of all sensors
	if (!ioctl(fd, ECOMPASS_IOC_GET_AFLAG, &flags)) {
		if (flags) {
			sensors |= SENSORS_ACCELERATION;
		} else {
			sensors &= ~SENSORS_ACCELERATION;
		}
	}
	if (!ioctl(fd, ECOMPASS_IOC_GET_MFLAG, &flags)) {
		if (flags) {
			sensors |= SENSORS_MAGNETIC_FIELD;
		} else {
			sensors &= ~SENSORS_MAGNETIC_FIELD;
		}
	}
	if (!ioctl(fd, ECOMPASS_IOC_GET_OFLAG, &flags)) {
		if (flags) {
			sensors |= SENSORS_ORIENTATION;
		} else {
			sensors &= ~SENSORS_ORIENTATION;
		}
	}

	return sensors;
}

static int data_pick_sensors(struct sensors_poll_context_t *dev, 
    sensors_event_t* data, int count)
{
	uint32_t mask = SUPPORTED_SENSORS;
	int num = 0;

	while (mask) {
		uint32_t i = 31 - __builtin_clz(mask);
		mask &= ~(1 << i);
		if (dev->pending_sensors & (1 << i)) {
			dev->pending_sensors &= ~(1 << i);
			data[num] = dev->sensors[i];
			LOGD_IF(0, "%s: %d-%d [%f, %f, %f], sensor: %d, type: %d", 
				__FUNCTION__, num, i,
				data[num].data[0], data[num].data[1], data[num].data[2],
				data[num].sensor, data[num].type);
			num++;
			if (num >= count) {
				goto FINISH;
			}
		}
	}

	if (num == 0) {
		LOGE("No sensor to return! pending_sensors=%08x", dev->pending_sensors);
		// we may end-up in a busy loop, slow things down, just in case.
		usleep(100000);
		return -1;
	}

FINISH:
	return num;
}

static int __control_activate(struct sensors_poll_device_t *device,
			int handle, int enabled)
{
	struct sensors_poll_context_t *dev;

	dev = (struct sensors_poll_context_t *)device;
	LOGD("+%s: handle=%d enabled=%d", __FUNCTION__, handle, enabled);

	if ((handle < SENSORS_HANDLE_BASE) || 
	    (handle >= SENSORS_HANDLE_BASE + SENSORS_SUPPORT_COUNT)) {
		return -1;
	}

	uint32_t mask = (1 << handle);
	uint32_t sensors = enabled ? mask : 0;
	
	uint32_t active = dev->active_sensors;
	uint32_t new_sensors = (active & ~mask) | (sensors & mask);
	uint32_t changed = active ^ new_sensors;
	if (!changed) {
		return 0;
	}

	int fd = control_open_ecs(dev);
	if (fd < 0) {
		return -1;
	}

	if (!active && new_sensors) {
		// force all sensors to be updated
		changed = SUPPORTED_SENSORS;
	}

	control_enable_disable_sensors(fd, new_sensors, changed);
	LOGD("Active sensors=%08x, real=%08x",
		new_sensors, control_read_sensors_state(fd));

	if (active && !new_sensors) {
		// close the driver
		control_close_ecs(dev);
	}
	dev->active_sensors = active = new_sensors;

    dev->sensors[handle].orientation.status = sSensorAccr[handle];

	return 0;
}

static int __control_set_delay(struct sensors_poll_device_t *device, 
	int handle, int64_t ns)
{
	struct sensors_poll_context_t *dev;
	dev = (struct sensors_poll_context_t *)device;
	LOGD("+%s: ns=%d", __FUNCTION__, (int)ns);

	if (dev->ecs_fd < 0) {
		return -1;
	}
	short delay = ns / 1000;
	if (!ioctl(dev->ecs_fd, ECOMPASS_IOC_SET_DELAY, &delay)) {
		return -errno;
	}

	return 0;
}

static int __data_poll(struct sensors_poll_device_t *device, 
	sensors_event_t* data, int count)
{
	struct sensors_poll_context_t *dev;
	dev = (struct sensors_poll_context_t *)device;

	int fd = dev->events_fd;
	if (fd < 0) {
		LOGE("Invalid file descriptor, fd=%d", fd);
		return -1;
	}
	if (data == NULL || count < 0) {
		LOGE("Invalid argument count=%d", count);
		return -1;
	}
	LOGD_IF(0, "%s, sensors_event: %p, count: %d",
		__FUNCTION__, data, count);

	// there are pending sensors, returns them now...
	if (dev->pending_sensors) {
		return data_pick_sensors(dev, data, count);
	}

	// wait until we get a complete event for an enabled sensor
	uint32_t new_sensors = 0;
	while (1) {
		/* read the next event */
		struct input_event event;
		if (read(fd, &event, sizeof(event)) != sizeof(event)) {
			continue;
		}
		uint32_t v;
		LOGD_IF(0, "type: %d code: %d value: %-5d time: %ds-%dns",
			event.type, event.code, event.value,
			(int)event.time.tv_sec, (int)event.time.tv_usec);
		if (event.type == EV_ABS) {
			switch (event.code) {
			#ifdef GSENSOR_XY_REVERT
			case EVENT_TYPE_ACCEL_Y:
				new_sensors |= SENSORS_ACCELERATION;
				dev->sensors[ID_A].acceleration.x = event.value * CONVERT_A_Y;
				break;
			case EVENT_TYPE_ACCEL_X:
				new_sensors |= SENSORS_ACCELERATION;
				dev->sensors[ID_A].acceleration.y = event.value * CONVERT_A_X;
				break;			
			#else
			case EVENT_TYPE_ACCEL_X:
				new_sensors |= SENSORS_ACCELERATION;
				dev->sensors[ID_A].acceleration.x = event.value * CONVERT_A_X;
				break;
			case EVENT_TYPE_ACCEL_Y:
				new_sensors |= SENSORS_ACCELERATION;
				dev->sensors[ID_A].acceleration.y = event.value * CONVERT_A_Y;
				break;
			#endif
			case EVENT_TYPE_ACCEL_Z:
				new_sensors |= SENSORS_ACCELERATION;
				dev->sensors[ID_A].acceleration.z = event.value * CONVERT_A_Z;
				break;

			case EVENT_TYPE_MAGV_X:
				new_sensors |= SENSORS_MAGNETIC_FIELD;
				dev->sensors[ID_M].magnetic.x = event.value * CONVERT_M_X;
				break;
			case EVENT_TYPE_MAGV_Y:
				new_sensors |= SENSORS_MAGNETIC_FIELD;
				dev->sensors[ID_M].magnetic.y = event.value * CONVERT_M_Y;
				break;
			case EVENT_TYPE_MAGV_Z:
				new_sensors |= SENSORS_MAGNETIC_FIELD;
				dev->sensors[ID_M].magnetic.z = event.value * CONVERT_M_Z;
				break;

			case EVENT_TYPE_ORIENT_YAW:
				new_sensors |= SENSORS_ORIENTATION;
				dev->sensors[ID_O].orientation.azimuth =  event.value * CONVERT_O_Y;
				break;
			case EVENT_TYPE_ORIENT_PITCH:
				new_sensors |= SENSORS_ORIENTATION;
				dev->sensors[ID_O].orientation.pitch = event.value * CONVERT_O_P;
				break;
			case EVENT_TYPE_ORIENT_ROLL:
				new_sensors |= SENSORS_ORIENTATION;
				dev->sensors[ID_O].orientation.roll = event.value * CONVERT_O_R;
				break;

			case EVENT_TYPE_ACCEL_STATUS:
				// accuracy of the acceleration
				dev->sensors[ID_A].acceleration.status = event.value;
				sSensorAccr[ID_A] = dev->sensors[ID_A].acceleration.status;
				break;
			case EVENT_TYPE_MAGV_STATUS:
				// accuracy of the magnetic sensor
				dev->sensors[ID_M].magnetic.status = event.value;
				sSensorAccr[ID_M] = dev->sensors[ID_M].magnetic.status;
				break;
			case EVENT_TYPE_ORIENT_STATUS:
				// accuracy of the ecompass calibration
				dev->sensors[ID_O].orientation.status = event.value;
				sSensorAccr[ID_O] = dev->sensors[ID_O].orientation.status;
				break;
			default:
				break;
			}
		} else if (event.type == EV_SYN) {
			if (event.code == SYN_CONFIG) {
				// we use SYN_CONFIG to signal that we need to exit the
				// main loop.
				//LOGD("got empty message: value=%d", event.value);
				return 0x7FFFFFFF;
			}
			if (new_sensors) {
				dev->pending_sensors = new_sensors;
				int64_t t = event.time.tv_sec * 1000000000LL +
						event.time.tv_usec * 1000;
				while (new_sensors) {
					uint32_t i = 31 - __builtin_clz(new_sensors);
					new_sensors &= ~(1 << i);
					dev->sensors[i].timestamp = t;
				}
				return data_pick_sensors(dev, data, count);
			}
		}
	}
}

static int __common_close(struct hw_device_t *device)
{
	struct sensors_poll_context_t *dev;

	dev = (struct sensors_poll_context_t *)device;
	if (dev) {
		if (dev->ecs_fd >= 0) {
			close(dev->ecs_fd);
			dev->ecs_fd = -1;
		}

		if (dev->events_fd >= 0) {
			close(dev->events_fd);
			dev->events_fd = -1;
		}

		free(dev);
	}

	return 0;
}

static int __module_methods_open(const struct hw_module_t *module,
		const char *id, struct hw_device_t **device)
{
	int res = -EINVAL;
	int i;

	if (strcmp(SENSORS_HARDWARE_POLL, id) == 0) {
		struct sensors_poll_context_t *dev;
		dev = (struct sensors_poll_context_t *)malloc(sizeof(*dev));
		if (!dev) {
			return res;
		}

		/* initialize our state here */
		memset(dev, 0, sizeof(*dev));

		dev->ecs_fd = -1;
		dev->events_fd = control_open_input(O_RDONLY);
		if (dev->events_fd < 0) {
			free(dev);
			return res;
		}
		
		/* initialize the procs */
		dev->device.common.tag = HARDWARE_DEVICE_TAG;
		dev->device.common.version = 0;
		dev->device.common.module = (struct hw_module_t *)module;
		dev->device.common.close = __common_close;

		dev->device.activate = __control_activate;
		dev->device.setDelay = __control_set_delay;
		dev->device.poll = __data_poll;

		*device = &dev->device.common;

		for (i = 0; i < SENSORS_SUPPORT_COUNT; i++) {
			dev->sensors[i].version = sizeof(struct sensors_event_t);
			dev->sensors[i].sensor = SENSORS_HANDLE_BASE + i;
			dev->sensors[i].type = i + 1;
		}

		res = 0;
	}

	return res;
}

static int __get_sensors_list(struct sensors_module_t* module,
			struct sensor_t const** list)
{
	*list = sensors_list;

	return (sizeof(sensors_list) / sizeof(struct sensor_t));
}


static struct hw_module_methods_t __module_methods = {
	open: __module_methods_open
};

struct sensors_module_t HAL_MODULE_INFO_SYM = {
	common: {
		tag: HARDWARE_MODULE_TAG,
		version_major: 2,
		version_minor: 0,
		id: SENSORS_HARDWARE_MODULE_ID,
		name: "MEMSIC Sensors Module",
		author: "MEMSIC Inc.",
		methods: &__module_methods,
		reserved: {0}
	},
	get_sensors_list: __get_sensors_list
};
