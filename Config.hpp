#ifndef __CONFIG__
#define __CONFIG__

#define __DEBUG__

// uncomment one to select the way of control
#define __GOBLE__      // to control pan/tilt steppers
//#define __NUNCHUK__    // to control pan/tilt steppers
#define __JOYSTCIK1__  // to control pan/tilt steppers
//
//#define __JOYSTCIK2__  // to control phone click servo & tilt stepper
//#define __BUTTON_PAD__ // to control phone click servo & tilt stepper

#ifdef __JOYSTCIK2__
#undef __BUTTON_PAD__
#endif
#ifdef __BUTTON_PAD__
#undef __JOYSTCIK2__
#endif

// uncomment one to select the of pan/tilt mechanism
#define __GIMBAL_PANORAMAS__
//#define __PAN_TILT_MECHANISM__
//#define __CAMERA_TURRET__

// uncomment one to select the control board
//#define __HUADUINO__
//#define __PAVO__
//#define __APUS__
#define __PRO_MINI__

#ifdef  __GOBLE__
#define GOBLE_BAUD_RATE 115200
//#define __GOBLE__SOFTWARE_SERIAL__
#endif

#define __TWO_ASIX_CONTROL__
#ifdef __TWO_ASIX_CONTROL__
#define BOTH_INDEX   0
#define TILT_INDEX   BOTH_INDEX
#define PAN_INDEX    BOTH_INDEX
#define ROTATE_INDEX 1 
#else
#define TILT_INDEX   0
#define PAN_INDEX    1
#define ROTATE_INDEX 2 
#endif

#endif //__CONFIG_
