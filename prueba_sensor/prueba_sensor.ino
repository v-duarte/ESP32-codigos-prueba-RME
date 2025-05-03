/*#include <HardwareSerial.h>

HardwareSerial serialSensor(1);

void setup() {
  Serial.begin(115200); // Inicia la comunicación serial con el monitor
  serialSensor.begin(115200, SERIAL_8N1, 16, 17); // Inicia la comunicación serial con el LiDAR

  Serial.println("Iniciando el LiDAR...");
}

void loop() {
  if (serialSensor.available()) {
    int distance = serialSensor.read(); // Lee la distancia del LiDAR
    Serial.print("Distancia: ");
    Serial.print(distance);
    Serial.println(" cm");
  }

  delay(1000); // Espera 1 segundo antes de la siguiente lectura
}
*/

#include <SoftwareSerial.h> 
#include <stdint.h>       
// TF-LC02 LiDAR

#define PORT_TX 12
#define PORT_RX 13

  
EspSoftwareSerial::UART PortLiDAR;  ////Define Pins Softserial donde esta conectado el LiDAR y GPIO12 como RX y GPIO13 como TX

uint8_t  cmd[5] = {0x55, 0xAA, 0x81, 0x00, 0xFA};  //Envia comando medir distancia

typedef struct { // Definicion de estructura para almacenar respuesta
  int distance;
  uint8_t  ErrorCode; 
  boolean receiveComplete;
} TF;

TF Lidar = {0,0,false};

void printRxArray(int rx[]) {
  Serial.print("Contenido de rx: ");
  for (int j = 0; j < 8; j++) {
    Serial.print(rx[j], HEX);  // Imprime cada byte en formato hexadecimal
    Serial.print(" ");          // Espacio entre los valores
  }
  Serial.println();  // Nueva línea al final
}


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
  if (PortLiDAR.available())
  {
    for (int i = 0; i < 8; i++) {
    rx[i] = PortLiDAR.read();
    }
    //rx[i] = PortLiDAR.read();
    //Serial.print("Byte leido:");  //Debug
    //Serial.println(rx[i]);        //Debug
    /*
    if (rx[0] != 0x55)
    {
      i = 0;
    } else if (i == 1 && rx[1] != 0xAA)
    {
      i = 0;
    } else if (i == 7)
    {
      Serial.print("Buffer:");
      printRxArray(rx);  // Imprime el contenido de rx cuando se reciben 8 bytes
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
  }*/
  
  Serial.print("Buffer:");
  printRxArray(rx);  // Imprime el contenido de rx cuando se reciben 8 bytes
  }
}

void setup() {
/********************************
  * Protocolo de puerto serie TOF: TTL
  * Velocidad de baudios: 115200
  * Bits de datos: 8
  * Bit de parada: 1
  * Paridad: Ninguna
  * Control de flujo: ninguno
 **************************************/
  Serial.begin(115200);       //Se aconseja usar baudrate alto , para evitar perdidas
  PortLiDAR.begin(115200, SWSERIAL_8N1, PORT_RX, PORT_TX, false);;
  delay (200); // El dispositivo comienza a enviar datos estables despues de 200ms despues del primer encendido.

}

void loop() {
  delay (100);
  PortLiDAR.write(cmd, 5);
  delay(33); // tiempo que tarda en responder el LiDAR
  getLidarData(&Lidar);
  //if (PortLiDAR.available()>0){   //Debug.
    //Serial.println("Hay datos para leer.");
  //}
  while (PortLiDAR.available()>0) {
    getLidarData(&Lidar);
  }
  Port_Print_Ascii(&Lidar);
  Lidar.receiveComplete = false;

  delay (1000);
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
