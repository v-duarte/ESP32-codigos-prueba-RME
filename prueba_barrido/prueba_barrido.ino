/*
  Rui Santos & Sara Santos - Random Nerd Tutorials
  Complete project details at https://RandomNerdTutorials.com/esp32-stepper-motor-28byj-48-uln2003/
  Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files.
  The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
  Based on Stepper Motor Control - one revolution by Tom Igoe
*/
#include <Stepper.h>

const int stepsPerRevolution = 2048;  // change this to fit the number of steps per revolution
const int stepsMedioBarrido = 128;  // change this to fit the number of steps per revolution
const int stepsBarrido = 256;  // change this to fit the number of steps per revolution
int contador = 1;


// ULN2003 Motor Driver Pins
#define IN1 15
#define IN2 14
#define IN3 2
#define IN4 16

// initialize the stepper library
Stepper myStepper(stepsPerRevolution, IN1, IN3, IN2, IN4);

void setup() {
  // set the speed at 5 rpm
  myStepper.setSpeed(5);
  // initialize the serial port
  Serial.begin(115200);
}

void loop() {
  if (contador > 0){
    // step one revolution in one direction:
  Serial.println("mediobarrido");
  myStepper.step(stepsMedioBarrido);
  //delay(1000);

  // step one revolution in the other direction:
  Serial.println("barrido");
  myStepper.step(-stepsBarrido);
  //delay(2000);
  contador --;
  }
}
