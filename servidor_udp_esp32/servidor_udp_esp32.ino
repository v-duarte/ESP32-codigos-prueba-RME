#include <WiFi.h>
#include <WiFiUdp.h>

const char* ssid = "UNLP_RC_Car";  // Nombre de la red
const char* password = "cetadrc01";  // Contraseña
IPAddress ipServidor;

const int udpPort = 1234;
WiFiUDP udp;

bool clienteConectado = false;
IPAddress ipCliente;
unsigned long ultimoMensaje = 0; // Tiempo del último mensaje recibido
const unsigned long timeout = 5000; // Tiempo de espera (2 segundos), lo subi a 5 por delays en pruebas.

int lastSequence = -1;  // Número de secuencia del último paquete procesado

char incomingPacket[255]; // Buffer para leer los paquetes UDP


void setup() {
  Serial.begin(115200);   //Comunicacion con la ciaa de 115200

  WiFi.softAP(ssid, password); // Iniciar AP

  udp.begin(udpPort); // Iniciar servidor UDP

  ipServidor = WiFi.softAPIP(); //Guardo mi ip

  //esp_sleep_enable_wifi_wakeup();  // Habilitar despertar por Wi-Fi

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
    Serial.println("Paquete descartado (llegó tarde o es duplicado)");
    return false;
  }
}

void loop() {
  //Serial.print("Clientes:");
  //Serial.println(WiFi.softAPgetStationNum());
  //{ // no es lo mismo que este en el AP a que este escuchando en UDP por lo tanto si no hay nadie puedo esp_light_sleep_start();// dormirme hasta que llegue una conexion.
    //Si hay alguien escuchando "UDP"
    // Enviar ip de ESP32 para que pueda mandar mensajes a travez de UDP
    //Esperar que me envie un mensaje de confirmacion o dsp de un timeout repetir proceso
    //si confirmo, establezco conexion con el dispositivo
    // Inicio conversacion con el dispositivo
    // Si envia que quiere conectarse a la ciaa, llamo al "conexion_ciaa" si devuelve true, le informo para que pueda enviar datos, Sino le indico que puede reintentar... y repito proceso.
    //Una vez conectado a la ciaa, recivo y paso datos a la ciaa, si se desconecta la aplicacion, informo a la ciaa 400 para terminar conexion.
    //Chequeo si hay alguien en AP, para establecer conexion UDP y sino termino la secion.
  //}
  
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
    Serial.println("No hay ningun cliente. Me duermo 5 segundos.");
    delay(5000);
    //Serial.println("No hay ningun cliente. Activo el modo light sleep.");
    //delay(30000);  // Esperar antes de dormir para leer el mensaje
    //esp_light_sleep_start();  //Me duermo en modo light sleep hasta detectar algun cliente.
  }
}

void procesar_paquete_udp(String datosPaquete) {
  datosPaquete.trim(); //Elimino posibles espacios en blanco, saltos y otros caracteres invisibles.
  if (datosPaquete == "200") {
    //conexion_ciaa();
    Serial.println("Me conecto a la CIAA.");
  } else if (datosPaquete == "400" || ipCliente == IPAddress(0, 0, 0, 0)) {
    //desconectar de la CIAA
    Serial.println("Me desconecto a la CIAA.");
    desconectar_cliente();
  } else {
    Serial.print("Envio a la CIAA:"); // Debug
    Serial.println(datosPaquete);  // Envio a la ciaa si no coincide con "200" o "400"
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
  lastSequence = -1; //Reinicio el numero de secuencia.
  //Serial.println("400");  //Deconecto la CIAA
  Serial.println("Desconectado de la CIAA.");
}