#include "esp_camera.h"
#include "WiFi.h"
#include "ESP32_FTPClient.h"
#include "time.h" // Para manejar el tiempo

const char* ssid_wifi = "Wifi";
const char* pass_wifi = "Wifi";

// Configuración del servidor FTP
char* ftp_server = "192.168.1.11";
char* work_dir = ".";

char* ftp_user = "user";
char* ftp_pass = "user";


// Definición de constantes para definir una zona horaria.
const int UTC_OFFSET_SECONDS = -10800; // UTC-3 (en segundos)
const int DAYLIGHT_OFFSET_SECONDS = 0; // Sin horario de verano


// FTP Client
ESP32_FTPClient ftp(ftp_server, ftp_user, ftp_pass);

// Pines de la cámara para ESP32-CAM (AI Thinker)
#define PWDN_GPIO_NUM     32
#define RESET_GPIO_NUM    -1
#define XCLK_GPIO_NUM      0
#define SIOD_GPIO_NUM     26
#define SIOC_GPIO_NUM     27

#define Y9_GPIO_NUM       35
#define Y8_GPIO_NUM       34
#define Y7_GPIO_NUM       39
#define Y6_GPIO_NUM       36
#define Y5_GPIO_NUM       21
#define Y4_GPIO_NUM       19
#define Y3_GPIO_NUM       18
#define Y2_GPIO_NUM        5
#define VSYNC_GPIO_NUM    25
#define HREF_GPIO_NUM     23
#define PCLK_GPIO_NUM     22

// Función para configurar la cámara
void setupCamera() {
  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;
  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;
  config.pin_sscb_sda = SIOD_GPIO_NUM;
  config.pin_sscb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_JPEG;

  if(psramFound()){
    config.frame_size = FRAMESIZE_VGA;  // Tamaño de la imagen (puedes cambiarlo)
    config.jpeg_quality = 10;           // Calidad JPEG
    config.fb_count = 2;
  } else {
    config.frame_size = FRAMESIZE_CIF;
    config.jpeg_quality = 12;
    config.fb_count = 1;
  }

  // Inicializar la cámara
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Error inicializando la cámara: 0x%x", err);
    return;
  }
}

// Función para conectarse al WiFi
void conectarWiFi() {
  const int maxIntentos = 10; // Número máximo de intentos
  int intentos = 0;

  Serial.println("Intentando conectar a WiFi...");
  WiFi.begin(ssid_wifi, pass_wifi);

  while (WiFi.status() != WL_CONNECTED && intentos < maxIntentos) {
    delay(500);
    Serial.print(".");
    intentos++;
  }

  if (WiFi.status() == WL_CONNECTED) {
    digitalWrite(33, LOW);
    Serial.println("\nConectado a WiFi");
    
    // Configurar NTP para sincronizar la hora
    configTime(UTC_OFFSET_SECONDS, DAYLIGHT_OFFSET_SECONDS, "pool.ntp.org");
    Serial.println("Sincronizando tiempo...");
    while (!time(nullptr)) {
      Serial.print(".");
      delay(500);
    }
    while (time(nullptr) < 1000000000) { // Verifica que el tiempo UNIX sea válido
      Serial.println("Esperando sincronización de tiempo...");
      delay(500);
    }
    Serial.println("\nTiempo sincronizado");
  } else {
    digitalWrite(33, HIGH);
    Serial.println("\nNo se pudo conectar a WiFi después de varios intentos.");
  }
}


// Función para generar un timestamp como nombre de archivo
String getTimestamp() {
  time_t now = time(nullptr);
  struct tm timeinfo;
  localtime_r(&now, &timeinfo);

  char buffer[30];
  strftime(buffer, sizeof(buffer), "%Y%m%d_%H%M%S.jpg", &timeinfo); // Formato: "20241217_154530.jpg"
  return String(buffer);
}

// Función para tomar una foto y enviarla al servidor FTP
void takeAndSendPhoto() {
  // Tomar foto
  camera_fb_t *fb = esp_camera_fb_get();
  if (!fb) {
    Serial.println("No se pudo capturar la foto");
    return;
  }

  // Obtener el nombre del archivo con timestamp
  String filename = getTimestamp();

  // Conectar al servidor FTP
  ftp.OpenConnection();
  Serial.println("Conectado al servidor FTP");

  // Subir la foto al FTP
  ftp.ChangeWorkDir(work_dir);  // Cambia al directorio de trabajo del servidor
  ftp.InitFile("Type I");  // Cambiar a modo binario
  ftp.NewFile(filename.c_str()); // Nombre del archivo en el servidor
  ftp.WriteData(fb->buf, fb->len);
  ftp.CloseFile();
  ftp.CloseConnection();

  // Liberar el frame buffer
  esp_camera_fb_return(fb);

  Serial.println("Foto enviada al servidor FTP con nombre: " + filename);
}

void setup() {

  pinMode(33, OUTPUT); // Configuro el pin como salida
  digitalWrite(33, HIGH);

  Serial.begin(115200);

  // Configurar cámara
  setupCamera();

  // Conectar a WiFi
  //conectarWiFi();
}

void loop() {
  // Toma una foto y envía cada 60 segundos
  if(WiFi.status() != WL_CONNECTED){
    conectarWiFi();
  }
  else{
    takeAndSendPhoto();
    delay(60000);
  }
}