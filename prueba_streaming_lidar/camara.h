#ifndef CAMARA_H
#define CAMARA_H

#include <Arduino.h>

void setup_camara(IPAddress ip);
void tomar_foto();
void alternar_modo();
void startCameraServer();

#endif