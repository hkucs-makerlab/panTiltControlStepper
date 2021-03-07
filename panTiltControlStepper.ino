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

#ifdef __PAN_TILT_MECHANISM__
// pan tilt mechanism
const float tiltStepperGearRatio = 48 / 12;
const float panStepperGearRatio = 69 / 11;
#else
// camera turret
const float tiltStepperGearRatio = 36 / 18;
const float panStepperGearRatio = 54 / 18;
#endif

boolean revX = false;
boolean revY = false;
const int panInterval = 10;
const int tiltInterval = 10;

TiltStepperMotor tiltStepper(tiltStepperGearRatio, TILT_IN1, TILT_IN2, TILT_IN3, TILT_IN4);
PanStepperMotor panStepper(panStepperGearRatio, PAN_IN1, PAN_IN2, PAN_IN3, PAN_IN4);

void setup() {
#ifdef __GOBLE__
  Goble.begin(GOBLE_BAUD_RATE);
#elif defined(__NUNCHUK__)
  nunchuk_init();
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
  static unsigned long prev_time = 0;
  static unsigned long prev_tilt_halt_time = 0;
  static unsigned long prev_pan_halt_time = 0;
  unsigned long cur_time;
  static char cmd[2] = {__HALT, __HALT};

  cur_time = millis();
#ifdef __GOBLE__
  check_goble(cmd);
#elif defined(__NUNCHUK__)
  check_nunchuk(cmd);
#endif
  if (cur_time - prev_time >= 200) {
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
    prev_time = cur_time;
  }
//
//  Console.println("cmd0:" + String(cmd[0]) + ", cmd1:" + String(cmd[1]));

}


#ifdef __NUNCHUK__
void check_nunchuk(char *cmd) {
  int joystickX, joystickY, switchState;
  String msg;

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
}
#endif

#ifdef __GOBLE__
void check_goble(char *cmd) {
  static long last_cmd_time = 0;

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
  } else  if (now - 1500 > last_cmd_time ) {
    cmd[0] = cmd[1] = __HALT;
  }
}
#endif
