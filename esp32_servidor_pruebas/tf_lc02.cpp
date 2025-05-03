// TF-LC02 LiDAR
#include <stdint.h> 
#include "tf_lc02.h" 
#define PORT_TX 12
#define PORT_RX 13

  

uint8_t  cmd[5] = {0x55, 0xAA, 0x81, 0x00, 0xFA};  //Envia comando medir distancia

typedef struct { // Definicion de estructura para almacenar respuesta
  int distance;
  uint8_t  ErrorCode; 
  bool receiveComplete;
} TF;

TF Lidar = {0,0,false};

/***************************************
 *  Protocolo:
 *  2 byte : Header 0x55 0xAA
 *  1 byte : Comando ：0x81 Unidad:mm
 *  1 byte : Longitud
 *  N byte : Configuracion parametros
 *  1 byte : Final del frame 0xFA
 *  ***************************************
 *  Ejemplo: para  medir distancia en MM  // VER EN EL MANUAL MAS COMANDOS
 *  Arduino Envia：55 AA 81 00 FA
 *  LiDAR TOF responde ： 55 AA 81 03 01 55 00 FA
 **************************************/
void getLidarData(TF* lidar) { // Procesa los bytes recibidos por el serial desde el LiDAR y los almacena en la estructura
  static char i = 0;
  static int rx[8];
  if (Serial1.available())
  {
    rx[i] = Serial1.read();
    if (rx[0] != 0x55)
    {
      i = 0;
    } else if (i == 1 && rx[1] != 0xAA)
    {
      i = 0;
    } else if (i == 7)
    {
      i = 0;
      if (rx[7] == 0xFA)
      {
        lidar->distance = rx[5] + rx[4] * 256;
        lidar->ErrorCode  = rx[6];
        lidar->receiveComplete = true;
      }
    } else
    {
      i++;
    }
  }
}

void setup_lidar() {
/********************************
  * Protocolo de puerto serie TOF: TTL
  * Velocidad de baudios: 115200
  * Bits de datos: 8
  * Bit de parada: 1
  * Paridad: Ninguna
  * Control de flujo: ninguno
 **************************************/
  Serial1.begin(115200, SERIAL_8N1, PORT_RX, PORT_TX);
  delay (200); // El dispositivo comienza a enviar datos estables despues de 200ms despues del primer encendido.
}

int getDistance(){
  Serial1.write(cmd, 5);
  delay(33); // tiempo que tarda en responder el LiDAR
  getLidarData(&Lidar);
  while (Serial1.available()>0) {
    getLidarData(&Lidar);
  }
  Lidar.receiveComplete = false;
  return (Lidar.distance);
}

void Port_Print_Ascii(TF* lidar)
{
  Serial.print("Distancia(mm):");
  Serial.println(lidar->distance);
  if(lidar->ErrorCode)
  {
    Serial.print("ErrorCode = ");
    Serial.println(lidar->ErrorCode,HEX);
  }
}
