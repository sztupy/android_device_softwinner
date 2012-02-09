/*****************************************************************************
 *  Copyright Statement:
 *  --------------------
 *  This software is protected by Copyright and the information and source code
 *  contained herein is confidential. The software including the source code
 *  may not be copied and the information contained herein may not be used or
 *  disclosed except with the written permission of MEMSIC Inc. (C) 2009
 *****************************************************************************/

/**
 * @file
 * @author  Robbie Cao<hjcao@memsic.cn>
 *          Dale Hou<byhou@memsic.cn>
 *
 * @brief
 * This file integrate MEMSIC 3-axis magnetic sensor combine with 2/3-axis 
 * acceleration sensor from major g-sensor vendor.<br>
 * The presence of memsicd is transparent to the user, application developer 
 * and node administrator. <br>
 * Its execution is automatically controlled by the initd system daemon.
 */

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/file.h>
#include <sys/ipc.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <dirent.h>
#include <stdio.h>
#include <string.h>
#include <syslog.h>
#include <signal.h>
#include <stdarg.h>
#include <time.h>
#include <errno.h>

#include <sensors_data_struct.h>
#include <sensors_acc_adapter.h>
#include <sensors_mag_adapter.h>
#include <sensors_algo_adapter.h>
#include <sensors_coordinate.h>

#define DEBUG			1

#if DEBUG
#define pr_trace(x...)	printf(x)
#else
#define pr_trace(x...)
#endif

#define MEMSICD_LOGBUFSZ	256			/*log buffer size*/
#define MEMSICD_LOGFILE		"/data/misc/sensors/memsicd.log"	/*log filename*/

#define ID_A				(0)
#define ID_M 				(1)
#define ID_O 				(2)

#define SENSORS_ACCELERATION		(1 << ID_A)
#define SENSORS_MAGNETIC_FIELD		(1 << ID_M)
#define SENSORS_ORIENTATION		(1 << ID_O)

/**
 * status of each sensor (from sensors.h)
 */

#define SENSOR_STATUS_UNRELIABLE	0
#define SENSOR_STATUS_ACCURACY_LOW	1
#define SENSOR_STATUS_ACCURACY_MEDIUM	2
#define SENSOR_STATUS_ACCURACY_HIGH	3

#define DAEMON_POLLING_INTV		20			// ms
#define DAEMON_NVM_STORE_INTV		(30 * 1000)		// ms

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

static float version[1] = {
#include "../version"
};

static struct algo_t *algo = NULL;
static struct device_acc_t *dev_acc = NULL;
static struct device_mag_t *dev_mag = NULL;
static int fd_acc = -1, fd_mag = -1, fd_ctrl = -1;

static int memsicd_log(const char *fmt, ...)
{
	va_list args;
	char buf[MEMSICD_LOGBUFSZ];
	int save_errno;
	struct tm *tm;
	time_t current_time;
	int fd_log;
	
	save_errno = errno;
	va_start(args, fmt);
	(void)time (&current_time); 		/* Get current time */
	tm = localtime (&current_time);
	sprintf(buf, "%02d/%02d %02d:%02d:%02d ", 
		tm->tm_mon + 1,
		tm->tm_mday, 
		tm->tm_hour, 
		tm->tm_min, 
		tm->tm_sec);
	
	vsprintf(buf + strlen(buf), fmt, args);
	va_end(args);

	fd_log = open(MEMSICD_LOGFILE, O_WRONLY | O_CREAT | O_APPEND, 0664);
	write(fd_log, buf, strlen(buf));
	close(fd_log);
	errno = save_errno;
	
	return 0;
}

static int memsicd_init(void)
{
	pid_t pid;

	/* parent exits , child continues */
	if ((pid = fork()) < 0) {
		return -1;
	} else if (pid != 0) {
		exit(0);
	}

	setsid();	/* become session leader */

#if (!DEBUG)
	close(0);	/* close STDIN */
	close(1);	/* close STDOUT */
	close(2);	/* close STDERR */
#endif

	umask(0); /* clear file mode creation mask */

	return 0;
}


static void memsicd_abort(void)
{
	memsicd_log("memsicd abort\n");
}

static void memsicd_sigterm(int signo)
{
	if (algo) {
		algo->nvm_store();
	}
	memsicd_abort();
	memsicd_log("signal: %d\n", signo);
	/* catched signal */
	memsicd_log("memsicd stopped\n");
	exit(0);
}

static int open_ctrl_dev(int mode)
{
	int fd;

	fd = open("/dev/ecompass_ctrl", mode);

	return fd;
}

static int control_read_sensors_state(int fd)
{
	if (fd < 0) {
		return 0;
	}

	short flags;
	int sensors = 0;

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

static int ecompass_init()
{
	return 0;
}

static int ecompass_poll(int state)
{
	int i;
	int res;
	int status;
	float mag_cald[3] = {0.0, 0.0, 0.0};
	float mag_off[3] = {0.0, 0.0, 0.0};
	static int val[12];

	struct SensorData_Raw raw;	// raw sensor data collection
	struct SensorData_Real real_d;	// real data for sensor device coordinate
	struct SensorData_Real real_a;	// real data for android coordinate
	struct SensorData_Real real_i;	// real data for ids coordinate
	struct SensorData_Algo sva;	// sensor vector for algorithm

	struct SensorData_Orientation orien;

	if (state & SENSORS_ACCELERATION || state & SENSORS_MAGNETIC_FIELD || state & SENSORS_ORIENTATION) {
		/* read raw data from acc-sensor */
		if(dev_acc->read_data(fd_acc, raw.acc)){
			return -1;
		}
		dev_acc->get_offset(fd_acc, raw.off_a);
		dev_acc->get_sensitivity(fd_acc, raw.sens_a);
		raw.dir_a = dev_acc->get_install_dir();

		// acceleration in unit G (GRAVITY_EARTH)
		real_d.acc[0] = ACC_UNIFY(raw.acc[0], raw.off_a[0], raw.sens_a[0]);
		real_d.acc[1] = ACC_UNIFY(raw.acc[1], raw.off_a[1], raw.sens_a[1]);
		real_d.acc[2] = ACC_UNIFY(raw.acc[2], raw.off_a[2], raw.sens_a[2]);

		// convert data to ids coordinate system
		coordinate_real_to_ids(real_i.acc, real_d.acc, raw.dir_a);

		// convert data to android coordinate system
		coordinate_real_to_android(real_a.acc, real_d.acc, raw.dir_a);

		val[0] = ACC_NORM2(real_a.acc[0]);
		val[1] = ACC_NORM2(real_a.acc[1]);
		val[2] = ACC_NORM2(real_a.acc[2]);
		val[3] = SENSOR_STATUS_ACCURACY_HIGH;
	}

	if (state & SENSORS_MAGNETIC_FIELD || state & SENSORS_ORIENTATION) {
		/* read raw data from mag-sensor */
		if (dev_mag->read_data(fd_mag, raw.mag)){
			return -1;
		}
		dev_mag->get_offset(fd_mag, raw.off_m);
		dev_mag->get_sensitivity(fd_mag, raw.sens_m);
		raw.dir_m = dev_mag->get_install_dir();

		// magnetic in unit Guass
		real_d.mag[0] = MAG_UNIFY(raw.mag[0], raw.off_m[0], raw.sens_m[0]);
		real_d.mag[1] = MAG_UNIFY(raw.mag[1], raw.off_m[1], raw.sens_m[1]);
		real_d.mag[2] = MAG_UNIFY(raw.mag[2], raw.off_m[2], raw.sens_m[2]);

		// convert data to ids coordinate system
		coordinate_real_to_ids(real_i.mag, real_d.mag, raw.dir_m);

		// convert data to android coordinate system
		coordinate_real_to_android(real_a.mag, real_d.mag, raw.dir_m);

		ids_degree_real_to_algo(&sva, &real_i);
		algo->calc_magcal_data(&sva, mag_cald);
		val[4] = MAG_NORM2(mag_cald[0]);
		val[5] = MAG_NORM2(mag_cald[1]);
		val[6] = MAG_NORM2(mag_cald[2]);
		val[7] = SENSOR_STATUS_ACCURACY_HIGH;
	}
	if (state & SENSORS_MAGNETIC_FIELD || state & SENSORS_ORIENTATION) {
		// prepare data for calibration
		ids_degree_real_to_algo(&sva, &real_i);
		// ecompass calibration
		algo->data_in(&sva);
		algo->calibrate(&sva);
		// calculate orientation
		algo->calc_orientation(&orien, &sva);

		val[8] = orien.azimuth;
		val[9] = orien.pitch;
		val[10] = orien.roll;
		val[11] = orien.quality;
	}

	res = ioctl(fd_ctrl, ECOMPASS_IOC_SET_YPR, val);
#if 0
	if (state & SENSORS_ACCELERATION || state & SENSORS_MAGNETIC_FIELD || state & SENSORS_ORIENTATION) {
		pr_trace("+Acc Raw Data, [x: %04d] [y: %04d] [z: %04d], [dir: %d]\n",raw.acc[0], raw.acc[1], raw.acc[2], raw.dir_a);
	}
	if (state & SENSORS_MAGNETIC_FIELD || state & SENSORS_ORIENTATION) {
		pr_trace("+Mag Raw Data, [x: %04d] [y: %04d] [z: %04d] [dir: %d]\n",raw.mag[0], raw.mag[1], raw.mag[2], raw.dir_m);
		pr_trace("+Real\t%2.3f\t%2.3f\t%2.3f\n",mag_cald[0], mag_cald[1], mag_cald[2]);
	}
#endif	

	return 0;
}

/*
 * main program of memsicd daemon
 */
int main(void)
{
	int stat_curr = 0, stat_prev = 0;
	int store_count = 0;

	if (memsicd_init() == -1) {
		memsicd_log("can't fork self\n");
		exit(0);
	}

	memsicd_log("memsicd started. version: %.2f\n", version[0]);
	signal(SIGTERM, memsicd_sigterm); /* arrange to catch the signal */

	signal(SIGHUP, memsicd_sigterm); /* arrange to catch the signal */
	signal(SIGINT, memsicd_sigterm); /* arrange to catch the signal */
	signal(SIGQUIT, memsicd_sigterm); /* arrange to catch the signal */
	signal(SIGTRAP, memsicd_sigterm); /* arrange to catch the signal */
	signal(SIGABRT, memsicd_sigterm); /* arrange to catch the signal */
	signal(SIGKILL, memsicd_sigterm); /* arrange to catch the signal */
	signal(SIGPIPE, memsicd_sigterm); /* arrange to catch the signal */
	signal(SIGSTOP, memsicd_sigterm); /* arrange to catch the signal */
	signal(SIGTSTP, memsicd_sigterm); /* arrange to catch the signal */
	signal(SIGURG, memsicd_sigterm); /* arrange to catch the signal */

	fd_ctrl = open_ctrl_dev(O_RDWR);
	pr_trace("ecs_ctrl fd: %d\n", fd_ctrl);
	if (fd_ctrl < 0) {
		memsicd_log("ecs_ctrl open failed\n");
		return -1;
	}

	dev_acc = sensors_get_acc_device();
	dev_acc->init();
	dev_mag = sensors_get_mag_device();
	dev_mag->init();
	algo = sensors_get_algorithm();
	// don't need algo init here

	fd_acc = dev_acc->open();
	pr_trace("fd_acc fd: %d\n", fd_acc);
	if (fd_acc < 0) {
		memsicd_log("open acc dev failed\n");
		return -1;
	}
	fd_mag = dev_mag->open();
	pr_trace("fd_mag fd: %d\n", fd_mag);
	if (fd_mag < 0) {
		memsicd_log("open mag dev failed\n");
		return -1;
	}

	ecompass_init();
	while (1) {
		struct timeval tv_begin;
		struct timeval tv_now;
		unsigned int usec_elapse = 0;	/* usec */ 

		gettimeofday (&tv_begin, NULL);

		stat_curr = control_read_sensors_state(fd_ctrl);
		if (!stat_prev && stat_curr) {
			// make sure init algo before open
			algo->init();
			algo->open();
			usleep(1000);
		} else if (stat_prev && !stat_curr) {
			algo->close();
			usleep(1000);
		}
		stat_prev = stat_curr;

		if (stat_curr) {
			ecompass_poll(stat_curr);

			store_count++;
			if (store_count >= (DAEMON_NVM_STORE_INTV / DAEMON_POLLING_INTV)) {
				algo->nvm_store();
				store_count = 0;
			}
		}

		if (algo->get_state()) {
			algo->clear_state();
			algo->restart();
		}

		gettimeofday (&tv_now, NULL);
		usec_elapse = (tv_now.tv_sec - tv_begin.tv_sec) * 1000000 + tv_now.tv_usec - tv_begin.tv_usec;
		//pr_trace("time:%d\n",usec_elapse);
		if (usec_elapse >= DAEMON_POLLING_INTV * 1000) {
			usleep(1000);
		} else {
			usleep(DAEMON_POLLING_INTV * 1000 - usec_elapse);
		}
	}

	return 0;
}

