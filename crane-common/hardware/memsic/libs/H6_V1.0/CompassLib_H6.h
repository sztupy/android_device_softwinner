/*****************************************************************************
 *  Copyright Statement:
 *  --------------------
 *  This software is protected by Copyright and the information and source code
 *  contained herein is confidential. The software including the source code
 *  may not be copied and the information contained herein may not be used or
 *  disclosed except with the written permission of MEMSIC Inc. (C) 2009
 *****************************************************************************/


#ifndef _MEMSIC_ECOMPASS_3D_H_
#define _MEMSIC_ECOMPASS_3D_H_

#ifdef __cplusplus
extern "C" {
#endif

//////////
//////////
//////////
//////////

#ifndef COMPASSLIB_TYPES
#define COMPASSLIB_TYPES
	#if defined __GNUC__
		// 8-bit data types
		#if __CHAR_BIT__ == 8
			typedef   signed char  int8; // signed 8-bit number    (-128 to +127)
			typedef unsigned char uint8; // unsigned 8-bit number  (+0 to +255)
		#else
			#error char is not 8-bits long!
		#endif // __CHAR_BIT__

		// 16-bit data types
		#if __SIZEOF_SHORT__ == 2
			typedef   signed short  int16; // signed 16-bt number    (-32,768 to +32,767)
			typedef unsigned short uint16; // unsigned 16-bit number (+0 to +65,535)
		#elif __SIZEOF_INT__ == 2
			typedef   signed int    int16; // signed 16-bt number    (-32,768 to +32,767)
			typedef unsigned int   uint16; // unsigned 16-bit number (+0 to +65,535)
		#elif __SIZEOF_LONG__ == 2
			typedef   signed long   int16; // signed 16-bt number    (-32,768 to +32,767)
			typedef unsigned long  uint16; // unsigned 16-bit number (+0 to +65,535)
		#else
		//	#error 16-bit data type not defined!
                        typedef   signed short  int16; // signed 16-bt number    (-32,768 to +32,767)
                        typedef unsigned short uint16; // unsigned 16-bit number (+0 to +65,535)
		#endif
		
		// 32-bit data types
		#if __SIZEOF_INT__ == 4
			typedef   signed int        int32; // signed 32-bit number   (-2,147,483,648 to +2,147,483,647)
			typedef unsigned int       uint32; // unsigned 32-bit number (+0 to +4,294,967,295)
		#elif __SIZEOF_LONG__ == 4
			typedef   signed long       int32; // signed 32-bit number   (-2,147,483,648 to +2,147,483,647)
			typedef unsigned long      uint32; // unsigned 32-bit number (+0 to +4,294,967,295)
		#elif __SIZEOF_LONG_LONG__ == 4
			typedef   signed long long  int32; // signed 32-bit number   (-2,147,483,648 to +2,147,483,647)
			typedef unsigned long long uint32; // unsigned 32-bit number (+0 to +4,294,967,295)
		#else
                        typedef   signed long       int32; // signed 32-bit number   (-2,147,483,648 to +2,147,483,647)
                        typedef unsigned long      uint32; // unsigned 32-bit number (+0 to +4,294,967,295)
		//	#error 32-bit data type not defined!
		#endif
	#elif defined __MSP430__
		typedef signed char int8;         // signed 8-bit number    (-128 to +127)
		typedef unsigned char uint8;      // unsigned 8-bit number  (+0 to +255)
		typedef signed int int16;         // signed 16-bt number    (-32,768 to +32,767)
		typedef unsigned int uint16;      // unsigned 16-bit number (+0 to +65,535)
		typedef signed long int int32;    // signed 32-bit number   (-2,147,483,648 to +2,147,483,647)
		typedef unsigned long int uint32; // unsigned 32-bit number (+0 to +4,294,967,295)
	#else
		#error Data types are not defined for this compiler/processor!
	#endif
#endif // COMPASSLIB_TYPES

// version of the compass library that this header represents

#define COMPASSLIB_H6_VERSION_HI 1
#define COMPASSLIB_H6_VERSION_LO 0

//////////
//////////
//////////
//////////

// macro to expand an angle in degrees to an internal 16-bit representation
// 0 to full scale represents a full 360 degrees of rotation
//    1 LSB = 360 degrees / 65536 = 0.00549 degrees
//
// $0000 = 0 degrees
// $4000 = 90 degrees
// $8000 = 180 degrees
// $C000 = 270 degrees
// $FFFF = 359.9945 degrees
//
// intended to be used as a compile time macro
#ifndef COMPASSLIB_ANGLE
#define COMPASSLIB_ANGLE(n)  ( (uint16) ( ( ( (n) / 360.0 ) * 65536 ) + 0.5 ) )
#endif // COMPASSLIB_ANGLE

//////////
//////////
//////////
//////////

// structures used by the library
//structure that output geomagnetic field value.
typedef struct {
	uint16 x,y,z;
}Geomagnetic_Offset_Value;

// structure that holds build date
typedef struct {
	uint8 Year, Month, Day, Hour, Minute, Second;
} COMPASSLIB_H6_BUILD_DATE;

// orientation
typedef struct
{
	int16 Roll;
	int16 Pitch;
	int16 Yaw;
} COMPASSLIB_H6_ORIENTATION;

// debugging information
typedef struct
{
	uint8 Data[12];
} COMPASSLIB_H6_DEBUG_INFO;

//////////
//////////
//////////
//////////

// possible return codes from library initialization
typedef enum {
	COMPASSLIB_H6_INIT_OK = 0x00,
	COMPASSLIB_H6_INIT_ERROR_BAD_STRUCT = 0x01,
	COMPASSLIB_H6_INIT_ERROR_WRONG_VERSION = 0x02,
	COMPASSLIB_H6_INIT_ERROR_NVM_CORRUPT = 0x04
} COMPASSLIB_H6_INIT_RESULT;

// enumerate library states
typedef enum
{
	COMPASSLIB_H6_STATE_UNAVAILABLE, // compass was not properly initialized and is unavailable
	COMPASSLIB_H6_STATE_OFF,         // compass is inactive - no features available
	COMPASSLIB_H6_STATE_ON,          // compass is active
	COMPASSLIB_H6_STATE_BUSY         // compass is active and is currently auto-calibrating - needs additional CPU cycles
} COMPASSLIB_H6_STATE;

//////////
//////////
//////////
//////////

// library initialization

// define a pointer to a function that performs reading from non-volatile memory
typedef uint16 (*COMPASSLIB_H6_NVM_READ_FUNCTION)(uint16 offset);

// define a pointer to a function that performs writing to non-volatile memory
typedef uint8 (*COMPASSLIB_H6_NVM_WRITE_FUNCTION)(uint16 offset, uint8 data);

// define the structure that must be passed to CompassLib_H6_Init()
typedef struct
{
	// size of this structure
	// helps library verify that proper version of structure is used in the application
	// user should pass the value sizeof(COMPASSLIB_H6_INIT_STRUCT)
	uint16 Size;

	// version number of the compass library
	// user should pass the values COMPASSLIB_H6_VERSION_HI and COMPASSLIB_H6_VERSION_LO
	uint8 VersionHi;
	uint8 VersionLo;

	// pointer to NVM read routine
	COMPASSLIB_H6_NVM_READ_FUNCTION NVM_Read;

	// pointer to NVM write routine
	COMPASSLIB_H6_NVM_WRITE_FUNCTION NVM_Write;
} COMPASSLIB_H6_INIT_STRUCT;

// call this function to initialize the library
COMPASSLIB_H6_INIT_RESULT CompassLib_H6_Init(const COMPASSLIB_H6_INIT_STRUCT * api);

//////////
//////////
//////////
//////////

// management

// task that performs processing during the application background
// should be called at least once per second, even if the library is disabled
COMPASSLIB_H6_STATE CompassLib_H6_Task(uint16 milliseconds);

// pass data into the compass library
void CompassLib_H6_DataIn(int16 gx, int16 gy, int16 gz, uint16 hx, uint16 hy, uint16 hz);

//////////
//////////
//////////
//////////

// control functions

// clear all calibration information stored by the compass library
// re-starts auto-calibration
void CompassLib_H6_ClearCalibration(void);

// used to enable/disable auto calibration features
// features are enabled at boot by default
void CompassLib_H6_EnableAutoCalibration(uint8 enable);

//////////
//////////
//////////
//////////

// status functions

// return the date/time that the library was built
COMPASSLIB_H6_BUILD_DATE CompassLib_H6_GetBuildDate(void);

// get the calibration quality factor (0 - 100%)
uint8 CompassLib_H6_GetCalibrationQualityFactor(void);

// returns number of bytes of non-volatile memory used by the library
uint16 CompassLib_H6_GetNVMBlockSize(void);

// indicates magnetic noise level (0 - 100%)
// 0 = no noise
// 100 = severe noise
uint8 CompassLib_H6_GetMagneticNoiseLevel(void);

// returns the current operating state of the library
COMPASSLIB_H6_STATE CompassLib_H6_GetOperatingState(void);

// library version information
// high-byte of result is major version
// low-byte of result is minor version
uint16 CompassLib_H6_GetVersion(void);

// returns whether auto calibration features are presently enabled
uint8 CompassLib_H6_IsAutoCalibrationEnabled(void);

// used for debugging
COMPASSLIB_H6_DEBUG_INFO CompassLib_H6_GetDebugInfo(void);

//////////
//////////
//////////
//////////

// gimballing

// function analyzes gravity vector (gx, gy, gz) and current magnetic enviroment (hx,hy,hz)
// and returns orientation
// magnetic information is passed in as raw LSBs, and is adjusted by to the current auto-calibration
COMPASSLIB_H6_ORIENTATION CompassLib_H6_CalculateOrientation(int16 gx, int16 gy, int16 gz, uint16 hx, uint16 hy, uint16 hz);
Geomagnetic_Offset_Value CompassLib_H6_Get_Offset(void);
#ifdef __cplusplus
}
#endif

#endif // _MEMSIC_ECOMPASS_3D_H_
