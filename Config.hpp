#ifndef __CONFIG__
#define __CONFIG__

#define __DEBUG__

// uncomment one to select the way of control
#define __BUTTON_PAD__
//#define __JOYSTCIK1__
//#define __JOYSTCIK2__
//#define __NUNCHUK__
#define __GOBLE__

#ifdef  __GOBLE__
#define GOBLE_BAUD_RATE 115200
//#define __GOBLE__SOFTWARE_SERIAL__
#endif
//
#define __GIMBAL_PANORAMAS__
//#define __PAN_TILT_MECHANISM__
//#define __CAMERA_TURRET__
//
#define __HUADUINO__
//#define __PAVO__
//#define __APUS__
//#define __PRO_MINI__

#endif //__CONFIG_
