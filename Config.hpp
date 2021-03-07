#ifndef __CONFIG__
#define __CONFIG__

#define __DEBUG__

// uncomment one to select the way of control
//#define __NUNCHUK__
#define __GOBLE__
#ifdef  __GOBLE__
#define GOBLE_BAUD_RATE 38400
//#define __GOBLE__SOFTWARE_SERIAL__
#endif
//
#define __PAN_TILT_MECHANISM__
//
#define __HUADUINO__
//#define __PAVO__
//#define __APUS__
//#define __PRO_MINI__

#ifdef __HUADUINO__
#define TILT_IN1  6
#define TILT_IN2  4
#define TILT_IN3  3
#define TILT_IN4  2
#define PAN_IN1   7
#define PAN_IN2   5
#define PAN_IN3   13
#define PAN_IN4   12
#elif defined(__PAVO__)
#define TILT_IN1  4
#define TILT_IN2  5
#define TILT_IN3  6
#define TILT_IN4  7
#define PAN_IN1   8
#define PAN_IN2   9
#define PAN_IN3   10
#define PAN_IN4   11
#elif defined(__APUS__)
#define TILT_IN1  2
#define TILT_IN2  3
#define TILT_IN3  4
#define TILT_IN4  7
#define PAN_IN1   8
#define PAN_IN2   11
#define PAN_IN3   12
#define PAN_IN4   13
#elif defined (__PRO_MINI__)
#define TILT_IN1  9
#define TILT_IN2  8
#define TILT_IN3  7
#define TILT_IN4  6
#define PAN_IN1   10
#define PAN_IN2   11
#define PAN_IN3   12
#define PAN_IN4   13
#endif



#endif //__CONFIG_
