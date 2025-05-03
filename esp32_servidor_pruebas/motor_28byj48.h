#ifndef MOTOR_28BYJ48_H
#define MOTOR_28BYJ48_H

#include <Arduino.h>

void setup_motor(int rpm);
void mover_motor(int pasos, int direccion); //Mueve el motor en una cantidad de pasos. La direccion a la que gira es 1 = derecha, 0 = izquierda.

#endif
