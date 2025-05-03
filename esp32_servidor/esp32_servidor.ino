#include <WiFi.h>
#include "camara.h"
#include "conexion_udp.h"

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

  //setup_camara(ipServidor);

}


bool conexion_ciaa(){
  bool ok= false;
  Serial.println("200");  //Mando un mensaje con 200 a la CIAA
  unsigned long startMillis = millis(); // Captura el tiempo de inicio
  unsigned long timeout = 5000; // Timeout de 5 segundos

  while (!ok && (millis() - startMillis < timeout)){
    if (Serial.available()){
      String mensaje = Serial.readStringUntil('\n'); //Lee hasta el salto de linea.
       if (mensaje == "200") {
        ok = true;
      }
    }
  }
  if (ok){
   return true; // Conexion establecida... 
  }
  return false; // Fallo la conexion.
}

void loop() {
  if (WiFi.softAPgetStationNum() > 0) {
    if(hay_paquete_udp()){
      if(!hay_cliente_conectado()){  //Nadie establecio conexion aun.
        conectar_cliente();
        conexion_ciaa();  //Conecto a la CIAA
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
}
/*
void barrido(){
  int medidas[32] = {};
  mover_motor(128, 1);  //Giro a la derecha para comenzar el barrrido
  for (int i=0; i < 31; i++){
    int aux = getDistance();
    if (aux < 8888){
      medidas[i] = getDistance()/10;  //Guardo la distancia que mide el sensor
    }
    else{
      medidas[i] = 255;
    }
    mover_motor(8, 0);  //Giro a la izquierda 8 pasos
  }
  medidas[31] = getDistance();
  mover_motor(120, 1);  //Giro a la derecha para comenzar el barrrido
}
*/