#ifndef MOTOR_28BYJ48_H
#define MOTOR_28BYJ48_H

// ULN2003 Motor Driver Pins
#define IN1 15
#define IN2 14
#define IN3 2
#define IN4 4

#include <Arduino.h>

void setup_motor(int rpm);
void mover_motor(int pasos, int direccion); //Mueve el motor en una cantidad de pasos. La direccion a la que gira es 1 = derecha, 0 = izquierda.

#endif
