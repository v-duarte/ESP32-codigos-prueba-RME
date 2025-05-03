#ifndef CONEXION_UDP_H
#define CONEXION_UDP_H

#include <Arduino.h>

void setup_udp(IPAddress ip);
bool hay_cliente_conectado();
bool hay_paquete_udp();
bool leer_udp();
void conectar_cliente();
void desconectar_cliente();
#endif