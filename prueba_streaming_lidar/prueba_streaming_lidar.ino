#include <WiFi.h>
#include <WiFiUdp.h>
#include "camara.h"
#include "tf_lc02.h"
#include "motor_28byj48.h"


const char* ssid = "Wifi";  // Nombre de la red
const char* password = "Wifi";  // Contraseña
IPAddress ipServidor;

const int udpPort = 1234;
WiFiUDP udp;

bool clienteConectado = false;
IPAddress ipCliente;
unsigned long ultimoMensaje = 0; // Tiempo del último mensaje recibido
const unsigned long timeout = 5000; // Tiempo de espera (2 segundos), lo subi a 5 por delays en pruebas.

uint32_t lastSequence = 0;  // Número de secuencia del último paquete procesado
uint32_t maxSeqNum = UINT32_MAX; // El máximo número de secuencia posible

char incomingPacket[255]; // Buffer para leer los paquetes UDP


void setup() {
  Serial.begin(115200);   //Comunicacion con la ciaa de 115200

  //WiFi.softAP(ssid, password); // Iniciar AP
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.println("Conectando a WiFi...");
  }
  Serial.println("Conectado a WiFi");


  //udp.begin(udpPort); // Iniciar servidor UDP

  ipServidor = WiFi.localIP(); //Guardo mi ip

  setup_camara(ipServidor);

  setup_motor(15);

  mover_motor(128, 1); //Mueve el motor en una cantidad de pasos. La direccion a la que gira es 1 = derecha, 0 = izquierda.
  setup_lidar();

}

/*
bool conexion_ciaa(){
  bool ok= false;
  Serial.println("200");  //Mando un mensaje con 200 a la CIAA
  unsigned long startMillis = millis(); // Captura el tiempo de inicio
  unsigned long timeout = 5000; // Timeout de 5 segundos

  while (!ok && (millis() - startMillis < timeout)){
    if (mySerial.available()){
      String mensaje = mySerial.readStringUntil('\n'); Lee hasta el salto de linea.
       if (mensaje == "200") {
        ok = true;
    }
  }
  if (ok){
   return true; // Conexion establecida... 
  }
  return false; // Fallo la conexion.
}
*/

bool hay_paquete_udp() {  //Prueba para detectar si hay paquetes junto comprobacion para descartar paquetes de clientes no autorizados.
  int packetSize = udp.parsePacket(); //revisa si llego un paquete, y si llego uno devuelve el tamaño.
  
  if (packetSize > 0 && clienteConectado && udp.remoteIP() != ipCliente) {
    // Si el paquete es de un cliente no autorizado, lo descartamos
    Serial.println("Paquete rechazado de cliente no autorizado: " + udp.remoteIP().toString());
    
    // Leer y descartar el paquete
    //udp.read(incomingPacket, sizeof(incomingPacket));
    udp.flush(); // O vaciar el buffer de paquetes, no se cual es mejor
    return false;  // Indicamos que no se debe procesar el paquete
  }
  return packetSize > 0;  // Si es un cliente autorizado, indicamos que hay paquete para procesar
}

bool leer_udp(){
  int len = udp.read(incomingPacket, 255);
  if (len > 0) {
    incomingPacket[len] = 0;  // Terminar el string
  }
  
  // Parsear el mensaje recibido (por ejemplo, "1,0,1")
  String packet = String(incomingPacket);
  int commaIndex = packet.indexOf(',');
  int sequenceNumber = packet.substring(0, commaIndex).toInt();  // Número de secuencia
  //String coordinates = packet.substring(commaIndex + 1);  // Coordenadas
  String datosPaquete = packet.substring(commaIndex + 1);   // String con el contenido del paquete sin el numero de secuencia.
  
  // Verificar si es un paquete más nuevo
  if (sequenceNumber > lastSequence) {
    lastSequence = sequenceNumber;  // Actualizar el último número de secuencia
    //Serial.println("Paquete válido recibido: " + coordinates);
    Serial.println("Paquete válido recibido: " + datosPaquete);
    procesar_paquete_udp(datosPaquete); //proceso el paquete
    return true;
  } else {
    if (lastSequence - sequenceNumber > (maxSeqNum / 2)) {  // Se ha reiniciado, actualizar el último número de secuencia
      lastSequence = sequenceNumber; // Reiniciar el seguimiento
      procesar_paquete_udp(datosPaquete); //proceso el paquete
      return true;
    }
    else{
      Serial.println("Paquete descartado (llegó tarde o es duplicado)");
      return false;
    }
  }
}

void loop() {
  /*
  if (WiFi.softAPgetStationNum() > 0) {
    if(hay_paquete_udp()){
      if(!clienteConectado){  //Nadie establecio conexion aun.
        conectar_cliente();
        //conexion_ciaa();  //Conecto a la CIAA
      }
      //Proceso el mensaje, ya que si hay_paquete_udp es true es porque o llego un paquete del cliente con el que hizo la conexion
      //O se establecio recientemente una nueva conexion.
      if(leer_udp()){ //Si es verdadero, significa que ya se proceso el mensaje
        ultimoMensaje = millis(); //Actualizo el registro del ultimo mensaje procesado
      }
    }
    else{ //No se detectaron paquetes UDP
      if (clienteConectado && (millis() - ultimoMensaje > timeout)) { //Desconecto cliente por timeout
          Serial.println("Desconecto de la CIAA por timeout.");
          desconectar_cliente();
      }
    }
  }
  else{
    if(clienteConectado){ //En caso de que se no haya nadie conectado a la red y no se haya cortado la conexion con el cliente adecuadamente
      desconectar_cliente();
    }
  }
  */
  Serial.print("Distancia(mm):");
  Serial.println(getDistance());
  delay(1000);  
}

void procesar_paquete_udp(String datosPaquete) {
  datosPaquete.trim();  // Elimino posibles espacios en blanco, saltos y otros caracteres invisibles.
  
  int codigo = 0;  // Inicializamos el código para el switch.

  if (datosPaquete == "200") {
    codigo = 200;
  } else if (datosPaquete == "400") {
    codigo = 400;
  } else if (datosPaquete == "foto") {
    codigo = 1000;  // Asigno un valor arbitrario para "foto".
  }

  switch (codigo) {
    case 200:
      Serial.println("Me conecto a la CIAA.");
      //conexion_ciaa();
      break;
    
    case 400:
      Serial.println("Me desconecto de la CIAA.");
      desconectar_cliente();
      break;

    case 1000:
      alternar_modo();  // Paso a modo para tomar fotos
      tomar_foto();
      alternar_modo();  // Paso a modo streaming
      break;

    default:
      Serial.print("Envio a la CIAA: ");  // Debug
      Serial.println(datosPaquete);  // Envío a la ciaa si no coincide con "200", "400" o "foto".
      break;
  }
}

void conectar_cliente(){
  ipCliente = udp.remoteIP(); //Guardo la IP del cliente
  clienteConectado = true;
  Serial.println("Cliente conectado: " + ipCliente.toString());
  // Enviar la IP del ESP32 al cliente
  udp.beginPacket(ipCliente, udpPort);
  String mensaje = "AP IP:" + ipServidor.toString();  // Construir el mensaje como String
  Serial.print("Envio al cliente:");
  Serial.println(mensaje);
  udp.write(reinterpret_cast<uint8_t*>(&mensaje), sizeof(mensaje));       // Enviar el mensaje como uint8_t*
  udp.endPacket(); // Enviar respuesta
  ultimoMensaje = millis(); //Para que no entre en el if de timeout en el primer mensaje
}

void desconectar_cliente(){
  ipCliente = IPAddress(0, 0, 0, 0);  // Asignar la IP "vacía" 0.0.0.0;
  clienteConectado = false;
  lastSequence = 0; //Reinicio el numero de secuencia.
  //Serial.println("400");  //Deconecto la CIAA
  Serial.println("Desconectado de la CIAA.");
}

void barrido(){
  int medidas[32];
  mover_motor(128, 1);  //Giro a la derecha para comenzar el barrrido
  for (i=0, i++, i <31){
    int aux = getDistance();
    if (aux < 8888)
      medidas[i] = getDistance()/10;  //Guardo la distancia que mide el sensor
    else
      medidas[i] = 255;
    mover_motor(8, 0);  //Giro a la izquierda 8 pasos
  }
  medidas[31] = getDistance();
  mover_motor(120, 1);  //Giro a la derecha para comenzar el barrrido
}
