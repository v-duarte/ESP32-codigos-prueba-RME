#include "camara.h"
#include "esp_camera.h"
#include <WiFi.h>
#include "esp_timer.h"
#include "img_converters.h"
#include "SD_MMC.h"            // Librería para trabajar con la tarjeta SD y Files
#include "fb_gfx.h"
#include "soc/soc.h"           // Disable brownout problems
#include "soc/rtc_cntl_reg.h"   // Disable brownout problems
#include "esp_http_server.h"
#include "pins_camara.h"

//Resolucion de la imagen
const int BMP_WIDTH = 800;
const int BMP_HEIGHT = 600;

const int POS_Y_LINEA = 85; // Fija la posición Y para dibujar la linea

httpd_handle_t camera_httpd = NULL; // Manejador del servidor HTTP
bool streaming = true;              // Variable para alternar entre streaming y captura de fotos

// Configuración de la cámara
void setup_camara(IPAddress ip) {
  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0); // Desactivar el brownout detector

  Serial.setDebugOutput(true);
  Serial.println();

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
  config.pixel_format = PIXFORMAT_JPEG; //Formato JPEG
  //config.pixel_format = PIXFORMAT_RGB565; //Formato RBG565

  if(psramFound()){
    config.frame_size = FRAMESIZE_SVGA;  // Resolución SVGA 800x600
    config.jpeg_quality = 10;            // Calidad del JPEG (0-63)
    config.fb_count = 2;                 // Número de buffers de framebuffer
  } else {
    config.frame_size = FRAMESIZE_SVGA;  // Resolución SVGA 800x600
    config.jpeg_quality = 12;            // Calidad del JPEG
    config.fb_count = 1;                 // Número de buffers de framebuffer
  }

  // Inicializar la cámara
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Error al inicializar la cámara: 0x%x", err);
    return;
  }

  // Configuración del Access Point (AP)
  IPAddress IP = ip;
  Serial.print("IP del Access Point: ");
  Serial.println(IP);
  // Inicializar la tarjeta SD, descomentar para usarla

  // if(!SD_MMC.begin()){
  //   Serial.println("Error al inicializar la tarjeta SD");
  //   return;
  // }

  // Iniciar el servidor de la cámara
  startCameraServer();

  Serial.print("Accede al streaming en: http://");
  Serial.print(IP);
  Serial.println("/stream");
}

//Toma una foto y lo guarda en el buffer fb
bool capturar_foto(camera_fb_t **fb) { // Capturar una foto
  *fb = esp_camera_fb_get();
  if (!*fb) {
    Serial.println("Error al capturar la foto");
    return false;
  }
  return true;
}

//Guarda la la imagen en la tarjeta SD
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

//Guarda la la imagen en la tarjeta SD
bool guardarFotoBMP (uint8_t * bmp_buf, size_t bmp_len, String path){  // Guardar la foto en la tarjeta SD
  bool exito;
  File file = SD_MMC.open(path.c_str(), FILE_WRITE);
  if (!file) {
    Serial.println("Error al abrir el archivo para escribir");
    exito=false;
  } else {
    file.write(bmp_buf, bmp_len);  // Escribir los datos de la imagen en el archivo: la imagen y el tamaño de archivo
    Serial.printf("Foto guardada: %s\n", path.c_str());
    exito=true;
  }

  file.close();  // Cerrar el archivo
  return exito;
}

void modificarPixelBMP(uint8_t * bmpData, int x, uint32_t color) {
    if (x < 0 || x >= BMP_WIDTH || POS_Y_LINEA < 0 || POS_Y_LINEA >= BMP_HEIGHT) {
        Serial.println("Coordenadas fuera de los límites de la imagen.");
        return;
    }

    uint8_t b = color & 0xFF;
    uint8_t g = (color >> 8) & 0xFF;
    uint8_t r = (color >> 16) & 0xFF;

    int rowSize = (BMP_WIDTH * 3 + 3) & ~3;
    uint8_t *pixelData = bmpData + 54; // Saltar encabezado BMP
    int pixelIndex = (BMP_HEIGHT - POS_Y_LINEA - 1) * rowSize + x * 3;

    if (pixelIndex + 2 >= BMP_HEIGHT * rowSize) {
        Serial.println("Índice fuera de rango en el buffer BMP.");
        return;
    }

    pixelData[pixelIndex] = b;
    pixelData[pixelIndex + 1] = g;
    pixelData[pixelIndex + 2] = r;
}


// Función para dibujar la línea de píxeles
void dibujarLinea(uint8_t * bmp_buf, uint32_t colores[32]) {
  for (int i = 0; i < 32; i++) {
    for (int j = 0; j<25; j++){
      int x = i*25 + j; // Posición X basada en el índice
      modificarPixelBMP(bmp_buf, x, colores[i]); // Dibujar píxel en la posición (x, y) con el color
    }
  }
}

// Método para capturar una foto y procesar imagen con las medidas tomadas.
// Devuelve 0 en caso de fallar el procesamiento, el tamaño del buffer en caso contrario
size_t tomarFoto(uint32_t colores[32], uint8_t **bmp_buf){
  camera_fb_t *fb;  //buffer de imagen  
  size_t bmp_len = 0;         // Tamaño del búfer BMP
  
  if(capturar_foto(&fb)){
    String path = "/foto" + String(millis()) + ".bmp";  // Crear nombre del archivo
    // Convierte la imagen JPEG a BMP
    if (!fmt2bmp(fb->buf, fb->len, fb->width, fb->height, fb->format, bmp_buf, &bmp_len)) {
        Serial.println("Error al convertir JPEG a BMP");
        esp_camera_fb_return(fb);
        return 0;
    }
    else {
      dibujarLinea(*bmp_buf, colores);
    }
  }
  // Liberar el frame buffer
  esp_camera_fb_return(fb);
  return bmp_len;
}

// Método para alternar entre los modos de streaming y captura de fotos
void alternar_modo() {
  streaming = !streaming;
  if (streaming) {
    Serial.println("Modo streaming activado");
    // Reiniciar el servidor para reiniciar el stream
    if (camera_httpd != NULL) {
      httpd_stop(camera_httpd);  // Detiene el servidor HTTP
    }
    startCameraServer();  // Reinicia el servidor de la cámara
  } else {
    Serial.println("Modo foto activado");
  }
}

esp_err_t stream_handler(httpd_req_t *req) {
  if (!streaming) {
    // Si no está en modo streaming, redirigir de nuevo al stream
    httpd_resp_set_status(req, "302 Found");
    httpd_resp_set_hdr(req, "Location", "/stream");  // Redirigir a /stream
    httpd_resp_send(req, NULL, 0);
    return ESP_OK;
  }

  camera_fb_t * fb = NULL;
  esp_err_t res = ESP_OK;
  size_t _jpg_buf_len = 0;
  uint8_t * _jpg_buf = NULL;
  char * part_buf[64];
  
  res = httpd_resp_set_type(req, "multipart/x-mixed-replace; boundary=frame");

  while (streaming) {
    fb = esp_camera_fb_get();
    if (!fb) {
      Serial.println("Error al capturar la imagen");
      res = ESP_FAIL;
    } else {
      _jpg_buf_len = fb->len;
      _jpg_buf = fb->buf;
    }

    if (res == ESP_OK) {
      size_t hlen = snprintf((char *)part_buf, 64, "Content-Type: image/jpeg\r\nContent-Length: %u\r\n\r\n", _jpg_buf_len);
      res = httpd_resp_send_chunk(req, (const char *)part_buf, hlen);
      res = httpd_resp_send_chunk(req, (const char *)_jpg_buf, _jpg_buf_len);
      res = httpd_resp_send_chunk(req, "\r\n--frame\r\n", 10);
    }

    esp_camera_fb_return(fb);
    
    if (res != ESP_OK) {
      break;
    }
  }
  return res;
}

// Función para iniciar el servidor de la cámara
void startCameraServer() {
  httpd_config_t config = HTTPD_DEFAULT_CONFIG();

  // Iniciar el servidor HTTP
  if (httpd_start(&camera_httpd, &config) == ESP_OK) {
    
    // Registrar la URI para el streaming
    httpd_uri_t stream_uri = {
      .uri       = "/stream",
      .method    = HTTP_GET,
      .handler   = stream_handler,
      .user_ctx  = NULL
    };
    httpd_register_uri_handler(camera_httpd, &stream_uri);
  }
}