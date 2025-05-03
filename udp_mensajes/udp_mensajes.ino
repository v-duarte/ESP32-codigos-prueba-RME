#include <WiFi.h>
#include <WiFiUdp.h>

// Configuraciones de red
const char* ssid = "UNLP_RC_CAR";        // Cambia por tu SSID
const char* password = "cetadrc01"; // Cambia por tu contraseña
#define BUFFER_SIZE 255 //Tamaño del buffer para mensajes UDP


// Configuraciones del servidor UDP
WiFiUDP Udp;
unsigned int localUdpPort = 4210;   // Puerto donde el Arduino recibirá los mensajes
char incomingPacket[BUFFER_SIZE];           // Buffer para recibir mensajes
char replyPacket[] = "Mensaje recibido";  // Respuesta del servidor

void setup() {
  Serial.begin(115200);
  
  // Conectarse al WiFi
  Serial.printf("Conectando a %s ", ssid);
  WiFi.begin(ssid, password);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  
  Serial.println("\nConectado al WiFi!");
  
  // Iniciar servidor UDP
  Udp.begin(localUdpPort);
  Serial.printf("Servidor UDP en puerto: %d\n", localUdpPort);
}

void loop() {
  int packetSize = Udp.parsePacket();
  if (packetSize) {
    // Recibe el paquete UDP
    int len = Udp.read(incomingPacket, BUFFER_SIZE); //udp.read guarda lo leido en el buffer incomingPacket y devuelve los bytes leidos.
    if (len > 0) {
      incomingPacket[len] = '\0';  // Asegurarse de que la cadena esté terminada
    }.
    
    Serial.printf("Paquete recibido de %s:%d\n", Udp.remoteIP().toString().c_str(), Udp.remotePort());
    Serial.printf("Contenido: %s\n", incomingPacket);
    
    // Procesar el mensaje recibido aquí según tu aplicación
    // ...
    
    // Enviar una respuesta de vuelta
    Udp.beginPacket(Udp.remoteIP(), Udp.remotePort());
    Udp.write(replyPacket);
    Udp.endPacket();
  }
}
