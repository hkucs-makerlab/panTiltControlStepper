#include "GoBLE.hpp"
#include "StepperMotor.hpp"
//#define __DEBUG__


//#define __SOFTWARE_SERIAL__
#ifdef __SOFTWARE_SERIAL__
#include <SoftwareSerial.h>
#define BAUD_RATE 9600
#define Console Serial
#define BT_RX_PIN 2
#define BT_TX_PIN 3
SoftwareSerial BlueTooth(BT_RX_PIN, BT_TX_PIN);
_GoBLE<SoftwareSerial, HardwareSerial> Goble(BlueTooth, Console);
#else
#define BAUD_RATE 38400
//#define BAUD_RATE 115200
#define Console Serial
#define BlueTooth Serial
_GoBLE<HardwareSerial, HardwareSerial> Goble(BlueTooth, Console);
#endif
//
#define __UPWARD 'f'
#define __DWONWARD 'b'
#define __LEFT 'l'
#define __RIGHT 'r'
#define __CENTER 'c'
#define __HALT 'h'

boolean revX = false;
boolean revY = false;
const int panInterval = 10;
const int tiltInterval = 10;

// pan tilt mechanism
//const float tiltStepperGearRatio = 48 / 12;
//const float panStepperGearRatio = 69 / 11;

// camera turret
const float tiltStepperGearRatio = 36 / 18;
const float panStepperGearRatio = 54 / 18;

//pavo
//TiltStepperMotor tiltStepper(tiltStepperGearRatio, 4, 5, 6, 7);
//PanStepperMotor panStepper(panStepperGearRatio, 8, 9, 10, 11);

//apus
//TiltStepperMotor tiltStepper(tiltStepperGearRatio, 2, 3, 4, 7);
//PanStepperMotor panStepper(panStepperGearRatio, 8, 11, 12, 13);

//huaduino
TiltStepperMotor tiltStepper(tiltStepperGearRatio, 6, 4, 3, 2);
PanStepperMotor panStepper(panStepperGearRatio, 7, 5, 13, 12);

//pro mini
//TiltStepperMotor tiltStepper(tiltStepperGearRatio, 9, 8, 7, 6);
//PanStepperMotor panStepper(panStepperGearRatio, 10, 11, 12, 13);

void setup() {
  Goble.begin(BAUD_RATE);
#ifdef __DEBUG__
#ifdef __SOFTWARE_SERIAL__
  Console.begin(115200);
#endif
  Console.println("in debugging mode");
#endif

#ifdef __DEBUG__
  Console.println("console started");
#endif
}

void loop() {

  //
  //  Console.println(tiltStepper.getPositionDeg());
  //  Console.println(panStepper.getPositionDeg());

  tiltStepper.run();
  panStepper.run();
  control();

}

void control() {
  static unsigned long prev_time = 0, prev_halt_time = 0;
  unsigned long cur_time;
  char cmd;

  cur_time = millis();
  cmd = check_goble();
  if (cur_time - prev_time >= 200) {
    prev_time = cur_time;
    switch (cmd) {
      case __UPWARD:
        tiltStepper.upward(tiltInterval);
        break;
      case __DWONWARD:
        tiltStepper.downward(tiltInterval);
        break;
      case __RIGHT:
        panStepper.right(panInterval);
        break;
      case __LEFT:
        panStepper.left(panInterval);
        break;
      case __CENTER:
        BlueTooth.end();
        tiltStepper.homePosition();
        panStepper.homePosition();
        BlueTooth.begin(BAUD_RATE);
        break;
      case __HALT:
        if (cur_time - prev_halt_time >= 3000) {
          prev_halt_time = cur_time;
          tiltStepper.haltSteppers();
          panStepper.haltSteppers();
        }
        break;
    }
#ifdef __DEBUG__
    Console.print("cmd: "); Console.println(cmd);
#endif
  }
}

char check_goble() {
  static char cmd = __HALT;

  int joystickX = 0;
  int joystickY = 0;

  if (Goble.available()) {
    joystickX = Goble.readJoystickX();
    joystickY = Goble.readJoystickY();

    if (joystickY > 190) {
      cmd = revY ? __UPWARD : __DWONWARD;
    } else if (joystickY < 80) {
      cmd = revY ? __DWONWARD : __UPWARD;
    } else if (joystickX > 190) {
      cmd = revX ?   __RIGHT : __LEFT;
    } else if (joystickX < 80) {
      cmd = revX ? __LEFT : __RIGHT;
    } else
      cmd = __HALT;

    if (Goble.readSwitchUp() == PRESSED) {
      cmd = revY ? __UPWARD : __DWONWARD;
    } else if (Goble.readSwitchDown() == PRESSED) {
      cmd = revY ? __DWONWARD : __UPWARD;
    } else if (Goble.readSwitchLeft() == PRESSED) {
      cmd = revX ? __LEFT : __RIGHT;
    } else if (Goble.readSwitchRight() == PRESSED) {
      cmd = revX ?   __RIGHT : __LEFT;
    } else if (Goble.readSwitchAction() == PRESSED) {
      cmd = __CENTER;
    } else if (Goble.readSwitchMid() == PRESSED) {
      cmd = __CENTER;
    } else if (Goble.readSwitchSelect() == PRESSED) {
      revY = !revY;
    } else if (Goble.readSwitchStart() == PRESSED) {
      revX = !revX;
    }
  }
  return cmd;
}
