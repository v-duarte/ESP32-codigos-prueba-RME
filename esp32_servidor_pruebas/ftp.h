#ifndef FTP_H
#define FTP_H

#include <Arduino.h>

void setup_ftp();
void subirFoto(uint8_t * buf, size_t buf_len);
void conectarWiFi();


#endif
