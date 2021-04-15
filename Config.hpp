#ifndef __CONFIG__
#define __CONFIG__

//#define __DEBUG__

// uncomment one to select the way of control
#define __GOBLE__      // to control pan/tilt steppers
#define __NUNCHUK__    // to control pan/tilt steppers
//#define __JOYSTCIK1__  // to control pan/tilt steppers
//#define __JOYSTCIK2__  // to control phone click servo & tilt stepper
//#define __BUTTON_PAD__ // to control phone click servo & tilt stepper

// uncomment one to select the of pan/tilt mechanism
#define __GIMBAL_PANORAMAS__
//#define __PAN_TILT_MECHANISM__
//#define __CAMERA_TURRET__

// uncomment one to select the control board
#define __HUADUINO__
//#define __PAVO__
//#define __APUS__
//#define __PRO_MINI__

#ifdef  __GOBLE__
#define GOBLE_BAUD_RATE 115200
//#define __GOBLE__SOFTWARE_SERIAL__
#endif
#endif //__CONFIG_
