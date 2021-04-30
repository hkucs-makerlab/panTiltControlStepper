#include "Config.hpp"
#include "StepperMotor.hpp"
#include "GPIOServo.hpp"

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
#define ROLL_PIN  10
#define JOYSTICK1_SWITCH_PIN 9
#define JOYSTICK2_SWITCH_PIN 8
#elif defined(__PAVO__)
#define TILT_IN1  4
#define TILT_IN2  5
#define TILT_IN3  6
#define TILT_IN4  7
#define PAN_IN1   8
#define PAN_IN2   9
#define PAN_IN3   10
#define PAN_IN4   11
#define ROLL_PIN  A2
#define JOYSTICK1_SWITCH_PIN  5
#elif defined(__APUS__)
#define TILT_IN1  2
#define TILT_IN2  3
#define TILT_IN3  4
#define TILT_IN4  7
#define PAN_IN1   8
#define PAN_IN2   11
#define PAN_IN3   12
#define PAN_IN4   13
#define ROLL_PIN  A2
#define JOYSTICK1_SWITCH_PIN  5
#elif defined (__PRO_MINI__)
#define TILT_IN1  9
#define TILT_IN2  8
#define TILT_IN3  7
#define TILT_IN4  6
#define PAN_IN1   10
#define PAN_IN2   11
#define PAN_IN3   12
#define PAN_IN4   13
#define ROLL_PIN  A2
#define JOYSTICK1_SWITCH_PIN  5
#endif

#ifdef __JOYSTCIK1__
#define JOYSTICK1_X_PIN  A4
#define JOYSTICK1_Y_PIN  A5
#endif
#ifdef __JOYSTCIK2__
#define JOYSTICK2_X_PIN  A2
#define JOYSTICK2_Y_PIN  A3
#endif

#ifdef __BUTTON_PAD__
#define BUTTON_PAD_PIN  A2
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
PhoneClickServo phoneClickServo(ROLL_PIN);

void setup() {
#ifdef __GOBLE__
  Goble.begin(GOBLE_BAUD_RATE);
#endif
#if defined(__NUNCHUK__)
  nunchuk_init();
#endif
#ifdef __JOYSTCIK1__
  pinMode(JOYSTICK1_SWITCH_PIN, INPUT_PULLUP);
#endif
#ifdef __JOYSTCIK2__
  pinMode(JOYSTICK2_SWITCH_PIN, INPUT_PULLUP);
#endif
  phoneClickServo.attach();
#ifdef __SOFTWARE_SERIAL__
  Console.begin(115200);
#endif
  Console.println("[started]");
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
#ifdef __TWO_ASIX_CONTROL__
  static char cmd[2] = {__HALT, __CENTER};
#else
  static char cmd[3] = {__HALT, __HALT, __CENTER};
#endif

  cur_time = millis();
#ifdef __GOBLE__
  rc = check_goble(cmd);
#endif
  if (!rc) {
#if defined(__NUNCHUK__)
    check_nunchuk(cmd);
#elif defined(__JOYSTCIK1__)
    check_joystick(cmd);
#endif

#if defined(__BUTTON_PAD__)
    check_button_pad(cmd);
#elif defined (__JOYSTCIK2__)
    check_joystick2(cmd);
#endif
  }

#ifdef __TWO_ASIX_CONTROL__
  switch (cmd[TILT_INDEX]) {
    case __UPWARD:
      tiltStepper.upward(tiltInterval);
      break;
    case __DOWNWARD:
      tiltStepper.downward(tiltInterval);
      break;
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
      if (cur_time - prev_tilt_halt_time >= 3000) {
        prev_tilt_halt_time = cur_time;
        tiltStepper.haltSteppers();
        panStepper.haltSteppers();
      }
      break;
  }
  //
#else
  switch (cmd[TILT_INDEX]) {
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
  switch (cmd[PAN_INDEX]) {
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
#endif //__TWO_ASIX_CONTROL__

  switch (cmd[ROTATE_INDEX]) {
    case __RIGHT:
      phoneClickServo.turnRight();
      break;
    case __LEFT:
      phoneClickServo.turnLeft();
      break;
    case __CENTER:
      phoneClickServo.center();
      break;
  }

}

#ifdef __BUTTON_PAD__
void check_button_pad( char *cmd) {
  static long last_check_time = 0;
  const int maxValue = 3;
  const int mid = maxValue / 2;
  static int count = mid;

  long now = millis();
  if (now - 150 > last_check_time) {
    last_check_time = now;
    int b = analogRead(BUTTON_PAD_PIN);
    if (b == 0) {
      count++;  // left
    } else if (b < 40) {
      cmd[TILT_INDEX] =  __DOWNWARD;// up
    } else if (b < 90) {
      cmd[TILT_INDEX] =   __UPWARD; // down
    } else if (b < 200) {
      count--; // right
    } else if (b < 400) {
      count = mid; // center
    } else {
      cmd[TILT_INDEX] = __HALT;
    }
    //
    if (count > maxValue - 1) {
      count = maxValue - 1;
    } if (count < 0) {
      count = 0;
    }
    switch (count) {
      case 0: cmd[ROTATE_INDEX] = __LEFT;
        break;
      case 1:  cmd[ROTATE_INDEX] = __CENTER;
        break;
      case 2: cmd[ROTATE_INDEX] = __RIGHT;
        break;
    }
  }

}
#endif

#ifdef __JOYSTCIK1__
void check_joystick( char *cmd) {
  static long last_check_time = 0;
  long now = millis();
  if (now - 200 > last_check_time) {

#ifdef __TWO_ASIX_CONTROL__
    int tilt = map(analogRead(JOYSTICK1_X_PIN), 0, 1023, 0, 255);
    int pan = map(analogRead(JOYSTICK1_Y_PIN), 0, 1023, 0, 255);
    if (tilt > 190) {
      cmd[TILT_INDEX] =  __UPWARD;
    } else if (tilt < 50) {
      cmd[TILT_INDEX] = __DOWNWARD ;
    } else if (pan > 190) {
      cmd[PAN_INDEX] = __LEFT;
    } else if (pan < 50) {
      cmd[PAN_INDEX] = __RIGHT ;
    } else  {
      cmd[BOTH_INDEX] = __HALT;
    }
#else
    int tilt = map(analogRead(JOYSTICK1_X_PIN), 0, 1023, 0, 255);
    if (tilt > 190) {
      cmd[TILT_INDEX] =  __UPWARD;
    } else if (tilt < 50) {
      cmd[TILT_INDEX] = __DOWNWARD ;
    } else {
      cmd[TILT_INDEX] = __HALT;
    }

    int pan = map(analogRead(JOYSTICK1_Y_PIN), 0, 1023, 0, 255);
    if (pan > 190) {
      cmd[PAN_INDEX] = __LEFT;
    } else if (pan < 50) {
      cmd[PAN_INDEX] = __RIGHT ;
    } else  {
      cmd[PAN_INDEX] = __HALT;
    }
#endif
    int center = digitalRead(JOYSTICK1_SWITCH_PIN);
    if (center == 0) {
      cmd[TILT_INDEX] = __CENTER;
      cmd[PAN_INDEX] = __CENTER;
    }
    last_check_time = now;
#ifdef __DEBUG__
    prn_cmds(cmd);
#endif
  }

}
#endif

#ifdef __JOYSTCIK2__
void check_joystick2(char *cmd) {
  static long last_check_time = 0;
  const int maxValue = 3;
  const int mid = maxValue / 2;
  static int count = mid;

  long now = millis();
  if (now - 300 > last_check_time) {
    last_check_time = now;
    int value = map(analogRead(JOYSTICK2_X_PIN), 0, 1023, 0, 255);
    if (value > 190) {
      cmd[TILT_INDEX] =  __UPWARD;
    } else if (value < 50) {
      cmd[TILT_INDEX] = __DOWNWARD ;
    } else {
      cmd[TILT_INDEX] = __HALT;
    }
    value = analogRead(JOYSTICK2_Y_PIN);
    if (value < 50) {
      count++;
    } else if (value > 1000) {
      count--;
    } else  if (digitalRead(JOYSTICK2_SWITCH_PIN) == 0) {
      count = mid;
    }
    if (count > maxValue - 1) {
      count = maxValue - 1;
    } if (count < 0) {
      count = 0;
    }
    switch (count) {
      case 0: cmd[ROTATE_INDEX] = __RIGHT ;
        break;
      case 1: cmd[ROTATE_INDEX] = __CENTER;
        break;
      case 2: cmd[ROTATE_INDEX] = __LEFT ;
        break;
    }
  }
}
#endif

#ifdef __NUNCHUK__
void check_nunchuk(char *cmd) {
  static long last_check_time = 0;
  int joystickX, joystickY, switchState;
  String msg;
  long now = millis();
  if (now - 200 > last_check_time) {
    last_check_time = now;
    if (!nunchuk_read()) {
      cmd[TILT_INDEX] = cmd[PAN_INDEX] = __HALT;
      return;
    }

    //nunchuk_print();
    if (nunchuk_buttonC() && nunchuk_buttonZ()) {
      cmd[TILT_INDEX] = __CENTER;
      cmd[PAN_INDEX] = __CENTER;
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

#ifdef __TWO_ASIX_CONTROL__
    if (joystickY > 210) {
      cmd[TILT_INDEX] = __UPWARD ;
    } else if (joystickY < 90) {
      cmd[TILT_INDEX] = __DOWNWARD;
    } else if (joystickX > 190) {
      cmd[PAN_INDEX] = __RIGHT;
    } else if (joystickX < 50) {
      cmd[PAN_INDEX] = __LEFT ;
    } else {
      cmd[BOTH_INDEX] = __HALT;
    }
#else
    if (joystickY > 210) {
      cmd[TILT_INDEX] = __UPWARD ;
    } else if (joystickY < 90) {
      cmd[TILT_INDEX] = __DOWNWARD;
    } else {
      cmd[TILT_INDEX] = __HALT;
    }
    if (joystickX > 190) {
      cmd[PAN_INDEX] = __RIGHT;
    } else if (joystickX < 50) {
      cmd[PAN_INDEX] = __LEFT ;
    } else {
      cmd[PAN_INDEX] = __HALT;
    }
#endif
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

#ifdef __TWO_ASIX_CONTROL__
    if (joystickY > 190) {
      cmd[TILT_INDEX] = revY ? __UPWARD : __DOWNWARD;
    } else if (joystickY < 80) {
      cmd[TILT_INDEX] = revY ? __DOWNWARD : __UPWARD;
    } else if (joystickX > 190) {
      cmd[PAN_INDEX] = revX ?   __RIGHT : __LEFT;
    } else if (joystickX < 80) {
      cmd[PAN_INDEX] = revX ? __LEFT : __RIGHT;
    } else {
      cmd[BOTH_INDEX] = __HALT;
    }
#else
    if (joystickY > 190) {
      cmd[TILT_INDEX] = revY ? __UPWARD : __DOWNWARD;
    } else if (joystickY < 80) {
      cmd[TILT_INDEX] = revY ? __DOWNWARD : __UPWARD;
    } else {
      cmd[TILT_INDEX] = __HALT;
    }
    //
    if (joystickX > 190) {
      cmd[PAN_INDEX] = revX ?   __RIGHT : __LEFT;
    } else if (joystickX < 80) {
      cmd[PAN_INDEX] = revX ? __LEFT : __RIGHT;
    } else {
      cmd[PAN_INDEX] = __HALT;
    }
#endif
    //
    if (Goble.readSwitchUp() == PRESSED) {
      cmd[TILT_INDEX] = revY ? __UPWARD : __DOWNWARD;
    } else if (Goble.readSwitchDown() == PRESSED) {
      cmd[TILT_INDEX] = revY ? __DOWNWARD : __UPWARD;
    }
    //
    if (Goble.readSwitchLeft() == PRESSED) {
      cmd[PAN_INDEX] = revX ? __LEFT : __RIGHT;
    } else if (Goble.readSwitchRight() == PRESSED) {
      cmd[PAN_INDEX] = revX ?   __RIGHT : __LEFT;
    }

    if (Goble.readSwitchAction() == PRESSED) {
      cmd[PAN_INDEX] = __CENTER;
      cmd[TILT_INDEX] = __CENTER;
    }
    //
    if (Goble.readSwitchPanLf() == PRESSED) {
      cmd[ROTATE_INDEX] = __LEFT;
    } else if (Goble.readSwitchPanRt() == PRESSED) {
      cmd[ROTATE_INDEX] = __RIGHT;
    } else if (Goble.readSwitchMid() == PRESSED) {
      cmd[ROTATE_INDEX] = __CENTER;
    }
    //
    if (Goble.readSwitchSelect() == PRESSED) {
      revY = !revY;
    } else if (Goble.readSwitchStart() == PRESSED) {
      revX = !revX;
    }
    //
    last_cmd_time = now;
    rc = true;
  } else  if (now - 1500 > last_cmd_time ) {
    cmd[TILT_INDEX] = cmd[PAN_INDEX] = __HALT;
  }
  return rc;
}
#endif
#ifdef __DEBUG__
void prn_cmds(char *cmd) {
  static long last_debug_time = 0;
  long now = millis();
  if (now - 1000 > last_debug_time) {
#ifdef __TWO_ASIX_CONTROL__
    Console.print("cmd0: "); Console.print(cmd[0]);
    Console.print(", cmd1: "); Console.println(cmd[1]);
#else
    Console.print("cmd0: "); Console.print(cmd[0]);
    Console.print(", cmd1: "); Console.print(cmd[1]);
    Console.print(", cmd2: "); Console.println(cmd[2]);
#endif
    last_debug_time = now;
  }
}
#endif
