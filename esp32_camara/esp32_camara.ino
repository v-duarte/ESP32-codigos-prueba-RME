#include "esp_camera.h"
#include "FS.h"               // Biblioteca para el sistema de archivos
#include "SD_MMC.h"           // Biblioteca para la tarjeta SD

// Pines para la cámara y la tarjeta SD en la ESP32-CAM (AI Thinker)
#define PWDN_GPIO_NUM     32
#define RESET_GPIO_NUM    -1
#define XCLK_GPIO_NUM     0
#define SIOD_GPIO_NUM     26
#define SIOC_GPIO_NUM     27
#define Y9_GPIO_NUM       35
#define Y8_GPIO_NUM       34
#define Y7_GPIO_NUM       39
#define Y6_GPIO_NUM       36
#define Y5_GPIO_NUM       21
#define Y4_GPIO_NUM       19
#define Y3_GPIO_NUM       18
#define Y2_GPIO_NUM       5
#define VSYNC_GPIO_NUM    25
#define HREF_GPIO_NUM     23
#define PCLK_GPIO_NUM     22

void setup() {
  Serial.begin(115200);
  
  // Configuración de la cámara
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
  
  // Inicialización de la cámara
  if (psramFound()) {
    config.frame_size = FRAMESIZE_UXGA;  // UXGA = 1600x1200
    config.jpeg_quality = 10;            // Calidad de JPEG
    config.fb_count = 2;                 // Número de frame buffers
  } else {
    config.frame_size = FRAMESIZE_SVGA;  // SVGA = 800x600
    config.jpeg_quality = 12;
    config.fb_count = 1;
  }
  
  // Inicialización de la cámara
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Error al inicializar la cámara: 0x%x\n", err);
    return;
  }

  // Inicialización de la tarjeta SD
  if (!SD_MMC.begin()) {
    Serial.println("Error al inicializar la tarjeta SD");
    return;
  }

  // Espacio disponible en la tarjeta SD
  uint8_t cardType = SD_MMC.cardType();
  if (cardType == CARD_NONE) {
    Serial.println("No se detecta ninguna tarjeta SD");
    return;
  }
}

bool capturar_foto(camera_fb_t **fb) { // Capturar una foto
  *fb = esp_camera_fb_get();
  if (!*fb) {
    Serial.println("Error al capturar la foto");
    return false;
  }
  return true;
}

bool guardar_foto(camera_fb_t *fb, String path){  // Guardar la foto en la tarjeta SD
  bool exito;
  File file = SD_MMC.open(path.c_str(), FILE_WRITE);
  if (!file) {
    Serial.println("Error al abrir el archivo para escribir");
    exito=false;
  } else {
    file.write(fb->buf, fb->len);  // Escribir los datos de la imagen en el archivo: la imagen y el tamaño de archivo
    Serial.printf("Foto guardada: %s\n", path.c_str());
    exito=true;
  }

  file.close();  // Cerrar el archivo
  return exito;
}

void tomar_foto(){
  camera_fb_t *fb;  //buffer de imagen
  if(capturar_foto(&fb)){
  // Crear nombre del archivo
  String path = "/foto" + String(millis()) + ".jpg";
  if(guardar_foto(fb, path)){
    //Podría llamar a que haga el barrido el sensor lidar o algo
  }
}
  // Liberar el frame buffer
  esp_camera_fb_return(fb);
}

void loop(){
  tomar_foto();
  // Pausa antes de la siguiente captura
  delay(5000);  // Captura una foto cada 5 segundos
}
