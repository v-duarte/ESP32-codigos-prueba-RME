#include "WiFi.h"
#include "time.h" // Para manejar el tiempo

const char* ssid_wifi = "";
const char* pass_wifi = "";


// Configuración del servidor FTP
char* ftp_server = "";

char* ftp_user = "";
char* ftp_pass = "";
char* ftp_dir = ".";

// Definición de constantes para definir una zona horaria.
const int UTC_OFFSET_SECONDS = -10800; // UTC-3 (en segundos)
const int DAYLIGHT_OFFSET_SECONDS = 0; // Sin horario de verano

const int maxIntentos = 10; // Número máximo de intentos para realizar una conexion inalambrica

// Función para generar un timestamp como nombre de archivo
String getTimestamp() {
  time_t now = time(nullptr);
  struct tm timeinfo;
  localtime_r(&now, &timeinfo);

  char buffer[30];
  strftime(buffer, sizeof(buffer), "%Y%m%d_%H%M%S.bmp", &timeinfo); // Formato: "20241217_154530.bmp"
  return String(buffer);
}

void setup() {
  // put your setup code here, to run once:
  pinMode(33, OUTPUT); // Configuro el pin como salida
  Serial.begin(115200);               //Comunicacion con la ciaa de 115200 esta en Serial=Usb

  int intentos = 0;

  WiFi.begin(ssid_wifi, pass_wifi);

  while (WiFi.status() != WL_CONNECTED && intentos < maxIntentos) {
    delay(500);
    intentos++;
  }

  if (WiFi.status() == WL_CONNECTED) {
    intentos = 0; //Reinicio el numero de intentos para usar la variable en la sincronizacion de hora
    // Configurar NTP para sincronizar la hora
    configTime(UTC_OFFSET_SECONDS, DAYLIGHT_OFFSET_SECONDS, "pool.ntp.org");
    while ((time(nullptr) < 1) && (intentos < maxIntentos)) {
      delay(500);
      intentos++;
    }
    if (time(nullptr) > 1000000000){  // Verifica si la hora es válida
      digitalWrite(33, LOW); // LED indica éxito
    } else {
      digitalWrite(33, HIGH); // Fallo en sincronización
    }
  } else {
    digitalWrite(33, HIGH); // Sin conexión WiFi
  }
}

void loop() {
  // put your main code here, to run repeatedly:
  if (WiFi.status() == WL_CONNECTED){
    Serial.println(getTimestamp());
    delay(10000);
  }
}
