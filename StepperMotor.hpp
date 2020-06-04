#ifndef STEPPER_MOTOR_H_
#define STEPPER_MOTOR_H_

#include <AccelStepper.h>

#define MAX_SPEED 900
#define MICRO_STEP 1

class StepperMotor : public AccelStepper {
  private:
    const int stepsPerRevolution = 2048; // 28BYJ-48 in full step mode
    float gearRatio;
    float radToStepFactor;
    uint8_t in1,in2,in3,in4;

  protected:
    long ratedStepsPerRevolution;


  public:
    StepperMotor(uint8_t in1, uint8_t in2, uint8_t in3, uint8_t in4):
      AccelStepper (AccelStepper::FULL4WIRE, in1, in3, in2, in4),
      gearRatio(1),
      ratedStepsPerRevolution(stepsPerRevolution * gearRatio) {
      setMaxSpeed(MAX_SPEED);
      this->in1=in1;
      this->in2=in2;
      this->in3=in3;
      this->in4=in4;
    }

    void setMaxSpeed(float maxSpeedValue) {
      AccelStepper::setMaxSpeed(maxSpeedValue);
      AccelStepper::setAcceleration(maxSpeedValue);
    }

    void setGearRatio(float ratio) {
      gearRatio = ratio;
      ratedStepsPerRevolution = stepsPerRevolution * gearRatio * MICRO_STEP;
      radToStepFactor = ratedStepsPerRevolution / 2 / PI;
    }

    float getPositionRad() const {
      return currentPosition() / radToStepFactor;
    }

    float getPositionDeg() const {
      return getPositionRad() * 180 / PI;
    }

    long getStepsPerRevolution() {
      return ratedStepsPerRevolution;
    }

    void moveByDegree(int degree) {
      boolean rev = degree < 0 ? true : false;
      if (rev) degree = -degree;
      float rad = degree * PI / 180;
      int steps = rad * radToStepFactor;
      if (rev) steps = -steps;
      move(steps);
    }

    void homePosition() {
      if (currentPosition() == 0) return;
      move(-currentPosition());
      runToPosition();
    }
    
    void halt() {
       digitalWrite(in1, LOW);
       digitalWrite(in2, LOW);
       digitalWrite(in3, LOW);
       digitalWrite(in4, LOW);
    }
};


class PanStepperMotor: public StepperMotor {
  private:

  public:
    PanStepperMotor(float gearRatio, uint8_t in1, uint8_t in2, uint8_t in3, uint8_t in4):
      StepperMotor (in1, in2, in3, in4) {
      setGearRatio(gearRatio);
    }

    void right(int degree) {
      moveByDegree(-degree);
    }

    void left(int degree) {
      moveByDegree(degree);
    }
};

class TiltStepperMotor: public StepperMotor {
  private:

  public:
    TiltStepperMotor(float gearRatio, uint8_t in1, uint8_t in2, uint8_t in3, uint8_t in4):
      StepperMotor (in1, in2, in3, in4) {
      setGearRatio(gearRatio);
    }

    void upward(int degree) {
      moveByDegree(-degree);
    }

    void downward(int degree) {
      moveByDegree(degree);
    }
};

#endif // STEPPER_MOTOR_H_
