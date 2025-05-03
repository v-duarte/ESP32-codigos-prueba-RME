#include "camara.h"
#include "WiFi.h"
#include "ESP32_FTPClient.h"

// Configuración WiFi
const char* ssid = "Tu_SSID";
const char* password = "Tu_Password";

// Configuración del servidor FTP
const char* ftp_server = "ftp.tu-servidor.com";
const char* ftp_user = "tu_usuario_ftp";
const char* ftp_pass = "tu_contraseña_ftp";

// FTP Client
ESP32_FTPClient ftp(ftp_server, ftp_user, ftp_pass);

// Función para conectarse al WiFi
void conectarWiFi() {
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("Conectado a WiFi");
}

// Función para conectarse al servidor FTP para enviar una foto al servidor 
void subirFoto(camera_fb_t *fb) {

  // Conectar al servidor FTP
  ftp.OpenConnection();
  Serial.println("Conectado al servidor FTP");

  // Subir la foto al FTP
  ftp.ChangeWorkDir("/");  // Cambia al directorio de trabajo del servidor
  ftp.InitFile("Type I");  // Cambiar a modo binario
  ftp.NewFile("foto.jpg"); // Nombre del archivo en el servidor
  ftp.WriteData(fb->buf, fb->len);
  ftp.CloseFile();
  ftp.CloseConnection();

  Serial.println("Foto enviada al servidor FTP");
}

void setup_ftp() {
  // Conectar a WiFi
  conectarWiFi();

}
