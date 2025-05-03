#include <WiFi.h>

const char* ssid = "UNLP_RC_CAR";
const char* password = "cetadrc01";

WiFiServer server(80);

int contador = 1;

void setup() {
  // Iniciar comunicación serial con el PC (Serial Monitor)
  Serial.begin(115200);

}

void loop() {
  String coordenadas;
  while(contador > 0){
    // Hacer girar ambos motores hacia adelante
    coordenadas = String(1.0) + "," + String(0.0);
    Serial.println(coordenadas);  // Enviar coordenadas por UART a la EDU-CIAA
    delay(1000);

    // Detengo motores
    coordenadas = String(0.0) + "," + String(0.0);
    Serial.println(coordenadas);
    delay(500);

    // Hacer girar ambos motores hacia atras
    coordenadas = String(-1.0) + "," + String(0.0);
    Serial.println(coordenadas);
    delay(1000);

    // Detengo motores
    coordenadas = String(0.0) + "," + String(0.0);
    Serial.println(coordenadas);
    delay(500);

    // Giro a la derecha
    coordenadas = String(1.0) + "," + String(0.5);
    Serial.println(coordenadas);
    delay(1000);

    // Detengo motores
    coordenadas = String(0.0) + "," + String(0.0);
    Serial.println(coordenadas);
    delay(500);

    // Giro a la izquierda
    coordenadas = String(0.75) + "," + String(-0.5);
    Serial.println(coordenadas);
    delay(1000);

    // Detengo motores
    coordenadas = String(0.0) + "," + String(0.0);
    Serial.println(coordenadas);
    delay(500);

    contador--;
  }
}
