#include <esp_heap_caps.h>

void setup() {
  Serial.begin(115200);
  
  // Verificar si PSRAM está habilitada
  if (psramInit()) {
    Serial.println("PSRAM está presente y habilitada.");
    Serial.print("PSRAM total: ");
    Serial.print(ESP.getPsramSize());
    Serial.println(" bytes");
  } else {
    Serial.println("PSRAM no está presente.");
  }
}

void loop() {
  // No hacer nada en el loop
}

