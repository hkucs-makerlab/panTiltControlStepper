#include "Config.hpp"
#include "StepperMotor.hpp"

#ifdef __NUNCHUK__
#include "Nunchuk.h"
#endif

#ifdef __GOBLE__
#include "GoBLE.hpp"
#ifdef __GOBLE__SOFTWARE_SERIAL__
#include <SoftwareSerial.h>
#define Console Serial
#define BT_RX_PIN A0
#define BT_TX_PIN A1
SoftwareSerial BlueTooth(BT_RX_PIN, BT_TX_PIN);
_GoBLE<SoftwareSerial, HardwareSerial> Goble(BlueTooth, Console);
#else // __SOFTWARE_SERIAL__
#define Console Serial
#define BlueTooth Serial
_GoBLE<HardwareSerial, HardwareSerial> Goble(BlueTooth, Console);
#endif // __SOFTWARE_SERIAL__
#else
#define Console Serial
#endif  //__GOBLE__
//
#define __UPWARD 'f'
#define __DOWNWARD 'b'
#define __LEFT 'l'
#define __RIGHT 'r'
#define __CENTER 'c'
#define __HALT 'h'

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


#ifdef __JOYSTCIK__
#define JOYSTICK_X_PIN       A4
#define JOYSTICK_Y_PIN       A5
#ifdef __HUADUINO__
#define JOYSTICK_SWITCH_PIN  9
#else
#define JOYSTICK_SWITCH_PIN  5
#endif //__HUADUINO__
#endif

#ifdef __PAN_TILT_MECHANISM__
#define TILT_GEAR_RATIO (48 / 12)
#define PAN_GEAR_RATIO  (69 / 11)
#elif defined(__CAMERA_TURRET__)
#define TILT_GEAR_RATIO (36 / 18)
#define PAN_GEAR_RATIO  (36 / 18)
#elif defined(__GIMBAL_PANORAMAS__)
#define TILT_GEAR_RATIO (40 / 10)
#define PAN_GEAR_RATIO  (40 / 10)
#else
#define TILT_GEAR_RATIO 1
#define PAN_GEAR_RATIO  1
#endif

boolean revX = false;
boolean revY = false;
const int panInterval = 10;
const int tiltInterval = 10;
const float tiltStepperGearRatio = TILT_GEAR_RATIO;
const float panStepperGearRatio = PAN_GEAR_RATIO;

TiltStepperMotor tiltStepper(tiltStepperGearRatio, TILT_IN1, TILT_IN2, TILT_IN3, TILT_IN4);
PanStepperMotor panStepper(panStepperGearRatio, PAN_IN1, PAN_IN2, PAN_IN3, PAN_IN4);

void setup() {
#ifdef __GOBLE__
  Goble.begin(GOBLE_BAUD_RATE);
#endif
#if defined(__NUNCHUK__)
  nunchuk_init();
#endif
#ifdef __JOYSTCIK__
  pinMode(JOYSTICK_SWITCH_PIN, INPUT_PULLUP);
#endif
#ifdef __SOFTWARE_SERIAL__
  Console.begin(115200);
#endif
  Console.println("started");
}

void loop() {
  //
  //  Console.println(tiltStepper.getPositionDeg());
  //  Console.println(panStepper.getPositionDeg());
  control();
  tiltStepper.run();
  panStepper.run();
}

void control() {
  static unsigned long prev_tilt_halt_time = 0;
  static unsigned long prev_pan_halt_time = 0;
  unsigned long cur_time;
  bool rc = false;
  static char cmd[2] = {__HALT, __HALT};

  cur_time = millis();
#ifdef __GOBLE__
  rc = check_goble(cmd);
#endif
  if (!rc) {
#if defined(__NUNCHUK__)
    check_nunchuk(cmd);
#elif defined(__JOYSTCIK__)
    check_joystick(cmd);
#endif
  }

  switch (cmd[0]) {
    case __UPWARD:
      tiltStepper.upward(tiltInterval);
      break;
    case __DOWNWARD:
      tiltStepper.downward(tiltInterval);
      break;
    case __HALT:
      if (cur_time - prev_tilt_halt_time >= 3000) {
        prev_tilt_halt_time = cur_time;
        tiltStepper.haltSteppers();
      }
      break;
  }
  //
  switch (cmd[1]) {
    case __RIGHT:
      panStepper.right(panInterval);
      break;
    case __LEFT:
      panStepper.left(panInterval);
      break;
    case __CENTER:
      tiltStepper.homePosition();
      panStepper.homePosition();
      break;
    case __HALT:
      if (cur_time - prev_pan_halt_time >= 3000) {
        prev_pan_halt_time = cur_time;
        panStepper.haltSteppers();
      }
      break;
  }
  //
  //  static unsigned long prev_time = 0;
  //  if (cur_time - prev_time >= 200) {
  //    prev_time = cur_time;
  //    Console.println("cmd0:" + String(cmd[0]) + ", cmd1:" + String(cmd[1]));
  //  }
}

#ifdef __JOYSTCIK__
void check_joystick( char *cmd) {
  static long last_joystick_time = 0;
  int tilt, pan, fire;
  long now = millis();
  if (now - 200 > last_joystick_time) {
    pan = map(analogRead(JOYSTICK_X_PIN), 0, 1023, 0, 255);
    if (pan > 190) {
      cmd[0] =  __UPWARD;
    } else if (pan < 50) {
      cmd[0] = __DOWNWARD ;
    } else {
      cmd[0] = __HALT;
    }

    tilt = map(analogRead(JOYSTICK_Y_PIN), 0, 1023, 0, 255);
    if (tilt > 190) {
      cmd[1] = __LEFT;
    } else if (tilt < 50) {
      cmd[1] = __RIGHT ;
    } else  {
      cmd[1] = __HALT;
    }

    fire = digitalRead(JOYSTICK_SWITCH_PIN);
    if (fire == 0) {
      cmd[0] = __CENTER;
      cmd[1] = __CENTER;
    }
    last_joystick_time = now;
  }
}
#endif

#ifdef __NUNCHUK__
void check_nunchuk(char *cmd) {
  static long last_nunchuk_time = 0;
  int joystickX, joystickY, switchState;
  String msg;
  long now = millis();
  if (now - 200 > last_nunchuk_time) {
    if (!nunchuk_read()) {
      cmd[0] = cmd[1] = __HALT;
      return;
    }

    //nunchuk_print();
    if (nunchuk_buttonC() && nunchuk_buttonZ()) {
      cmd[0] = __CENTER;
      cmd[1] = __CENTER;
      return;
    }

    if (nunchuk_buttonZ()) {
      //Console.println("Pressed button Z");
      float pitch_angle = nunchuk_pitch() * 180 / M_PI;
      if (pitch_angle >= -40 && pitch_angle <= 90) {
        joystickY = map(pitch_angle, 60, -90, 255, 0);
      } else {
        joystickY = 128;
      }
      float roll_angle = nunchuk_roll() * 180 / M_PI;
      if (roll_angle >= -90 && roll_angle <= 90) {
        joystickX = map(roll_angle, -90, 90, 0, 255);
      } else {
        joystickY = 128;
      }
      //    msg = "Pitch: " + String(pitch_angle) + ", Roll: " + String(roll_angle);
      //    Console.println(msg);
    } else {
      joystickX = map(nunchuk_joystickX(), -128, 127, 0, 255);
      joystickY = map(nunchuk_joystickY(), -128, 127, 0, 255);
    }
    //msg = "X: " + String(joystickX) + " ,Y:" + String(joystickY);
    //Console.println(msg);
    if (joystickY > 210) {
      cmd[0] = __UPWARD ;
    } else if (joystickY < 90) {
      cmd[0] = __DOWNWARD;
    } else {
      cmd[0] = __HALT;
    }
    if (joystickX > 190) {
      cmd[1] = __RIGHT;
    } else if (joystickX < 50) {
      cmd[1] = __LEFT ;
    } else {
      cmd[1] = __HALT;
    }
    last_nunchuk_time = now;
  }
}
#endif

#ifdef __GOBLE__
bool check_goble(char *cmd) {
  static long last_cmd_time = 0;
  bool rc = false;
  int joystickX = 0;
  int joystickY = 0;

  long now = millis();
  if (Goble.available()) {
    joystickX = Goble.readJoystickX();
    joystickY = Goble.readJoystickY();

    if (joystickY > 190) {
      cmd[0] = revY ? __UPWARD : __DOWNWARD;
    } else if (joystickY < 80) {
      cmd[0] = revY ? __DOWNWARD : __UPWARD;
    } else {
      cmd[0] = __HALT;
    }
    //
    if (joystickX > 190) {
      cmd[1] = revX ?   __RIGHT : __LEFT;
    } else if (joystickX < 80) {
      cmd[1] = revX ? __LEFT : __RIGHT;
    } else {
      cmd[1] = __HALT;
    }

    if (Goble.readSwitchUp() == PRESSED) {
      cmd[0] = revY ? __UPWARD : __DOWNWARD;
    } else if (Goble.readSwitchDown() == PRESSED) {
      cmd[0] = revY ? __DOWNWARD : __UPWARD;
    }
    //
    if (Goble.readSwitchLeft() == PRESSED) {
      cmd[1] = revX ? __LEFT : __RIGHT;
    } else if (Goble.readSwitchRight() == PRESSED) {
      cmd[1] = revX ?   __RIGHT : __LEFT;
    } else if (Goble.readSwitchAction() == PRESSED) {
      cmd[1] = __CENTER;
    } else if (Goble.readSwitchMid() == PRESSED) {
      cmd[1] = __CENTER;
    }
    //
    if (Goble.readSwitchSelect() == PRESSED) {
      revY = !revY;
    } else if (Goble.readSwitchStart() == PRESSED) {
      revX = !revX;
    }
    last_cmd_time = now;
    rc = true;
  } else  if (now - 1500 > last_cmd_time ) {
    cmd[0] = cmd[1] = __HALT;
  }
  return rc;
}
#endif
