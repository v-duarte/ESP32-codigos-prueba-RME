#include <WiFi.h>
#include <WiFiUdp.h>

const char* ssid = "UNLP_RC_CAR";
const char* password = "cetadrc01";
IPAddress IP;
WiFiUDP Udp;
unsigned int localUdpPort = 4210; // Puerto UDP para escuchar mensajes

unsigned long tiempoActivo; //timer para timeout
const unsigned long tiempoTimeout = 30000; //Timeout de 30 segundos
char incomingPacket[255]; // Buffer para mensajes UDP entrantes

enum EstadoAplicacion {
  CONECTADO = 0,  //Cliente conectado, ESP32 establecio conexion con la CIAA
  WARNING_UART = 1,
  ERROR_UART = 2, //ESP32 no pudo conectarse con la CIAA
  DESCONECTADO = 3  //Cliente desconectado, ESP32 ordena a la CIAA que este en modo de bajo consumo.
};

EstadoAplicacion estado;

void conectar_uart(){
  int reintentos = 0;
  estado = WARNING_UART;
  while (reintentos < 3){
    Serial.println("200");  // Mando un mensaje con 200 a la CIAA
    if(Serial.available() > 0){ // Si la CIAA responde
      String mensaje = Serial.readStringUntil('\n');
      if (mensaje == "200"){
        estado = CONECTADO;
        break;
      }
    }
    else{
      reintentos++;
      delay(100);
    }
  }
  if (reintentos > 2){
    estado = ERROR_UART;
  }
}

void setup() {
  Serial.begin(115200);
  conectar_uart(); // Intento establecer conexión con la CIAA
  
  // Iniciar WiFi en modo AP
  WiFi.softAP(ssid, password);
  IP = WiFi.softAPIP();
  
  // Iniciar servidor UDP
  Udp.begin(localUdpPort);
  Serial.printf("Servidor UDP escuchando en IP: %s, puerto: %d\n", IP.toString().c_str(), localUdpPort);
  
  tiempoActivo = millis();
}

bool ciaa_dormida = false; // Control de estado de la CIAA

void enviar_datos_wifi(const char* mensaje, IPAddress clienteIP, int puertoCliente) {
  Udp.beginPacket(clienteIP, puertoCliente); // Iniciar paquete UDP
  Udp.write(mensaje);  // Escribir el mensaje
  Udp.endPacket();  // Enviar el paquete
}

bool hay_datos_wifi() {
  int packetSize = Udp.parsePacket();
  return packetSize > 0;
}

float leer_joystick_X() {
  // Simular la lectura del joystick en X
  return random(0, 1024) / 1024.0;
}

float leer_joystick_Y() {
  // Simular la lectura del joystick en Y
  return random(0, 1024) / 1024.0;
}

void loop() {
  if (WiFi.softAPgetStationNum() > 0) { // Verificar si hay clientes conectados
    switch (estado) {
      case CONECTADO:
        while (WiFi.softAPgetStationNum() > 0) {
          if (hay_datos_wifi()) {
            // Procesar el paquete recibido
            int packetSize = Udp.read(incomingPacket, 255); // Leer el paquete UDP
            incomingPacket[packetSize] = '\0';  // Asegurarse de que sea una cadena válida
            Serial.printf("Mensaje recibido: %s\n", incomingPacket);

            // Leer el joystick y enviar las coordenadas a la CIAA
            float pos_X = leer_joystick_X();
            float pos_Y = leer_joystick_Y();
            String coordenadas = String(pos_X) + "," + String(pos_Y);
            Serial.println(coordenadas);  // Enviar coordenadas por UART a la EDU-CIAA
            enviar_datos_wifi(coordenadas.c_str(), Udp.remoteIP(), Udp.remotePort()); // Enviar respuesta al cliente

            tiempoActivo = millis();  // Actualizar el tiempo de actividad
          } else {
            // Si no hay datos, enviar coordenadas 0,0 (detener el coche)
            String coordenadas = String(0.0) + "," + String(0.0);
            Serial.println(coordenadas);  // Enviar coordenadas 0,0 a la CIAA
            if (millis() - tiempoActivo >= tiempoTimeout) {
              WiFi.softAPdisconnect(true);  // Forzar la desconexión si no hay actividad
              estado = DESCONECTADO;
            }
          }
        }
        break;

      case DESCONECTADO:
        conectar_uart();
        if (estado == CONECTADO) {
          tiempoActivo = millis();
        }
        break;

      case ERROR_UART:
        conectar_uart();
        break;
    }
  } else { // No hay clientes conectados
    if (estado != DESCONECTADO) {
      estado = DESCONECTADO;
    }
  }
}
