#ifndef CAMARA_H
#define CAMARA_H

#include <Arduino.h>

void setup_camara(IPAddress ip);
void tomar_foto();
size_t tomarFoto(uint32_t colores[32], uint8_t **bmp_buf);
void alternar_modo();
void startCameraServer();

#endif