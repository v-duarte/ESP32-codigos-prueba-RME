#include "esp_camera.h"
#include "WiFi.h"
#include "ESP32_FTPClient.h"
#include "time.h" // Para manejar el tiempo

const char* ssid_wifi = "Wifi";
const char* pass_wifi = "Wifi";

// Configuración del servidor FTP
char* ftp_server = "192.168.1.5";

char* ftp_user = "user";
char* ftp_pass = "user";

// Definición de constantes para definir una zona horaria.
const int UTC_OFFSET_SECONDS = -10800; // UTC-3 (en segundos)
const int DAYLIGHT_OFFSET_SECONDS = 0; // Sin horario de verano


// FTP Client
ESP32_FTPClient ftp(ftp_server, ftp_user, ftp_pass);

// Función para generar un timestamp como nombre de archivo
String getTimestamp() {
  time_t now = time(nullptr);
  struct tm timeinfo;
  localtime_r(&now, &timeinfo);

  char buffer[30];
  strftime(buffer, sizeof(buffer), "%Y%m%d_%H%M%S.jpg", &timeinfo); // Formato: "20241217_154530.jpg"
  return String(buffer);
}

// Función para conectarse al WiFi
void conectarWiFi() {
  const int maxIntentos = 10; // Número máximo de intentos
  int intentos = 0;

  WiFi.begin(ssid_wifi, pass_wifi);

  while (WiFi.status() != WL_CONNECTED && intentos < maxIntentos) {
    delay(500);
    intentos++;
  }

  if (WiFi.status() == WL_CONNECTED) {
    digitalWrite(33, LOW);  //Enciendo el LED integrado del ESP32 para indicar que esta conectado a Internet.
    
    // Configurar NTP para sincronizar la hora
    configTime(UTC_OFFSET_SECONDS, DAYLIGHT_OFFSET_SECONDS, "pool.ntp.org");
    while (!time(nullptr)) {
      delay(500);
    }
    while (time(nullptr) < 1000000000) { // Verifica que el tiempo UNIX sea válido
      delay(500);
    }
  } else {
    digitalWrite(33, HIGH); //Apago el LED integrado del ESP32 para indicar que esta no conectado a Internet.
  }
}

// Función para enviar la foto al servidor FTP
void subirFoto(uint8_t * buf, size_t buf_len) {
  // Si tiene conexion a WiFi, sube la foto al servidor FTP
  if(WiFi.status() == WL_CONNECTED){
    // Obtener el nombre del archivo con timestamp
    String filename = getTimestamp();

    // Conectar al servidor FTP
    ftp.OpenConnection();

    // Subir la foto al FTP
    ftp.ChangeWorkDir("/");  // Cambia al directorio de trabajo del servidor
    ftp.InitFile("Type I");  // Cambiar a modo binario
    ftp.NewFile(filename.c_str()); // Nombre del archivo en el servidor
    ftp.WriteData(buf, buf_len);
    ftp.CloseFile();
    ftp.CloseConnection();
  }
}

void setup_ftp() {

  pinMode(33, OUTPUT); // Configuro el pin como salida

}