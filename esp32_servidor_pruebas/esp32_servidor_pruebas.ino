#include <WiFi.h>
#include "camara.h"
#include "conexion_udp.h"
#include "tf_lc02.h"
#include "motor_28byj48.h"
#include "ftp.h"

const char* ssid = "UNLP_RC_Car";  // Nombre de la red
const char* password = "cetadrc01";  // Contraseña
IPAddress ipServidor;



unsigned long ultimoMensaje = 0; // Tiempo del último mensaje recibido
const unsigned long timeout = 5000; // Tiempo de espera (2 segundos), lo subi a 5 por delays en pruebas.

void setup() {
  Serial.begin(115200);   //Comunicacion con la ciaa de 115200

  WiFi.softAP(ssid, password); // Iniciar AP

  ipServidor = WiFi.softAPIP(); //Guardo mi ip

  setup_udp(ipServidor);

  setup_camara(ipServidor);
  
  setup_lidar();
  setup_motor(15); //Se inicia el motor paso a paso a 15 RPM

  setup_ftp();

  delay(1000);
  
  //pruebaBarrido();
}

void loop() {
  /*
  if (WiFi.softAPgetStationNum() > 0) {
    Serial.println("");
    if(hay_paquete_udp()){
      if(!hay_cliente_conectado()){  //Nadie establecio conexion aun.
        conectar_cliente();
        //conexion_ciaa();  //Conecto a la CIAA
        ultimoMensaje = millis(); //Para que no entre en el if de timeout en el primer mensaje
      }
      //Proceso el mensaje, ya que si hay_paquete_udp es true es porque o llego un paquete del cliente con el que hizo la conexion
      //O se establecio recientemente una nueva conexion.
      if(leer_udp()){ //Si es verdadero, significa que ya se proceso el mensaje
        ultimoMensaje = millis(); //Actualizo el registro del ultimo mensaje procesado
      }
    }
    else{ //No se detectaron paquetes UDP
      if (hay_cliente_conectado() && (millis() - ultimoMensaje > timeout)) { //Desconecto cliente por timeout
          Serial.println("Desconecto de la CIAA por timeout.");
          desconectar_cliente();
      }
    }
  }
  else{
    if(hay_cliente_conectado()){ //En caso de que se no haya nadie conectado a la red y no se haya cortado la conexion con el cliente adecuadamente
      desconectar_cliente();
    }
  }
  */
  /*
  Serial.println("Inicia el barrido.");
  barrido();
  Serial.println("Termina el barrido.");
  */
  //delay(10000);

  barrido();
  //pruebaBarrido();
  delay(30000);
}

// void barrido(){
//   int medidas[32] = {};
//   mover_motor(128, 1);  //Giro a la derecha para comenzar el barrrido
//   for (int i=0; i < 31; i++){
//     int dist = getDistance();
//     if (dist < 8888){
//       medidas[i] = getDistance()/10;  //Guardo la distancia que mide el sensor
//     }
//     else{
//       medidas[i] = 255;
//     }
//     Serial.print("Distancia:");
//     Serial.println(medidas[i]);
//     mover_motor(8, 0);  //Giro a la izquierda 8 pasos
//   }
//   medidas[31] = getDistance();
//   mover_motor(120, 1);  //Giro a la derecha para comenzar el barrrido
// }

void barrido(){
  uint8_t *bmp_buf = NULL;    // Búfer que contendrá los datos BMP
  uint32_t medidas[32] = {};  //Aqui se guardaran las medidas en formato 0xRRGGBB.

  if(WiFi.status() != WL_CONNECTED){  // Revisa si tiene conexion a WiFi
    conectarWiFi();
  }

  if(WiFi.status() == WL_CONNECTED){  // Si la tiene, se realiza el barrido
    mover_motor(128, 1);  //Giro a la derecha para comenzar el barrrido
    for (int i=0; i < 31; i++){
      medidas[31 - i] = mapearColor(getDistance());  //Guardo el color que corresponde a la distancia que mide el sensor.
      mover_motor(8, 0);  //Giro a la izquierda 8 pasos
    }
    medidas[0] = mapearColor(getDistance());
    mover_motor(120, 1);  //Giro a la derecha 120 pasos para centrar el sensor.
    size_t buf_len = tomarFoto(medidas, &bmp_buf); 
    if(buf_len > 0){  //Si buf_len = 0, se traduce como false en el if. true si buf_len > 0.
      delay(500);
      subirFoto(bmp_buf, buf_len);
    }
    free(bmp_buf);  //Libero la memoria ocupada por el buffer una vez que termine
  }
  else{
    //Si es posible, informar a la aplicacion que no se realizo la operacion
    return; //Por el momento, solo sale del metodo.
  }
}

// void pruebaBarrido(){
//   uint8_t *bmp_buf = NULL;    // Búfer que contendrá los datos BMP
//   uint32_t medidas[32] = {};
//   for (int i=0; i < 32; i++){
//     //Serial.print("Distancia:");
//     //Serial.println(i);
//     medidas[31- i] = mapearColor(i*100);  //Guardo el color que corresponde a la distancia que mide el sensor.
//     //En i=0, medidas[31] = 0 -> Rojo puro. A medida que i aumenta, la distancia aumenta. Entonces en medidas[0]=3100.
//     //Serial.print("Indice:");
//     //Serial.print(i);
//     //Serial.print(" ");
//     //Serial.printf("Color: 0x%06X\n", medidas[31- i]);
//   }
//   //tomarFoto(medidas); Viejo
//   size_t buf_len = tomarFoto(medidas, &bmp_buf);  
//   if(buf_len > 0){  //Si buf_len = 0, se traduce como false en el if. true si buf_len > 0.
//     delay(1000);
//     subirFoto(bmp_buf, buf_len);
//   }
//   free(bmp_buf);  //Libero la memoria ocupada por el buffer una vez que termine
// }

uint32_t mapearColor(int distancia) { // Función para mapear distancia en milímetros a color RGB
  //No usa funcion de mapeo
  int colorDistancia = constrain(distancia/10, 0, 255);   // Me aseguro que el color equivalente a la distancia esté dentro del rango 0-255

  // Mapear distancia a valores RGB (rojo cercano, verde lejano)
  int red = 255 - colorDistancia;
  int blue = 0; // Mantener en 0 para un gradiente rojo-verde
  int green = colorDistancia;

  // Empaquetar el color en formato 0xRRGGBB
  return (red << 16) | (green << 8) | blue;
}
