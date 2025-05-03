const int udpPort = 1234;
WiFiUDP udp;

uint32_t lastSequence = 0;  // Número de secuencia del último paquete procesado
uint32_t maxSeqNum = UINT32_MAX; // El máximo número de secuencia posible