#include <Stepper.h>
#include "motor_28byj48.h"

const int stepsPerRevolution = 2048;  // change this to fit the number of steps per revolution

// initialize the stepper library
Stepper myStepper(stepsPerRevolution, IN1, IN3, IN2, IN4);

void setup_motor(int rpm) {
  // set the speed at 5 rpm
  myStepper.setSpeed(rpm);
}

void mover_motor(int pasos, int direccion){
  if (direccion)
    myStepper.step(pasos);
  else
    myStepper.step(-pasos);
}