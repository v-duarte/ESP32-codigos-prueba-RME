// ESP32-CAM con LiDAR TF-LC02

// Pines de UART1 del ESP32-CAM
#define RX_PIN 12  // Defino el GPIO12 como el pin RX 
#define TX_PIN 13  // Defino el GPIO13 como el pin TX

uint8_t cmd[5] = {0x55, 0xAA, 0x81, 0x00, 0xFA};  // Comando para medir distancia

typedef struct { // Estructura para almacenar la respuesta
  int distance;
  uint8_t ErrorCode; 
  boolean receiveComplete;
} TF;

TF Lidar = {0, 0, false};

/***************************************
 * Protocolo del LiDAR:
 * 2 byte : Header 0x55 0xAA
 * 1 byte : Comando ：0x81 Unidad:mm
 * 1 byte : Longitud
 * N byte : Configuración de parámetros
 * 1 byte : Final del frame 0xFA
 **************************************/
void getLidarData(TF* lidar) { // Procesar los bytes recibidos por el serial
  char i = 0; // Indice para recorrer rx.
  static int rx[8];
  while (Serial1.available() > 0) { //Leo todos los bytes del mensaje
    rx[i] = Serial1.read();
    i++;
  }

  if (rx[0] != 0x55 || rx[1] != 0xAA || rx[7] != 0xFA) {  //Reviso hubo algun error en el formato del header o fin de paquete.
      lidar->ErrorCode = 0xFF;  //0xFF indica que hubo un error en el paquete en si, no en los datos. 
  } 
  else{
    lidar->distance = rx[5] + rx[4] * 256;  //rx[4] es el byte más significativo (multiplicado por 256) y rx[5] es el byte menos significativo.
    lidar->ErrorCode = rx[6];
    lidar->receiveComplete = true;
  }
}

void setup() {
  // Inicializar Serial para depuración
  Serial.begin(115200);  // Baudrate alto para evitar pérdidas
  
  // Inicializar UART1 para LiDAR en ESP32-CAM. Este UART1 es nativo del ESP32-CAM.
  Serial1.begin(115200, SERIAL_8N1, RX_PIN, TX_PIN);
}

void loop() {
  delay(100);
  Serial1.write(cmd, 5);  // Enviar comando para medir distancia
  delay(33);  // Tiempo de espera para la respuesta del LiDAR

  getLidarData(&Lidar);

  Port_Print_Ascii(&Lidar);  // Mostrar los datos recibidos
  Lidar.receiveComplete = false;

  delay(1000);
}

void Port_Print_Ascii(TF* lidar) {
  Serial.print("Distancia (mm): ");
  Serial.println(lidar->distance);
  
  if (lidar->ErrorCode) {
    Serial.print("Código de error = ");
    Serial.println(lidar->ErrorCode, HEX);
  }
}
