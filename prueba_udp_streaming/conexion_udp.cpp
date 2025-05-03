uint32_t lastSequence = 0;  // Número de secuencia del último paquete procesado
uint32_t maxSeqNum = UINT32_MAX; // El máximo número de secuencia posible

bool clienteConectado = false;

bool hay_cliente_conectado(){
  return clienteConectado;
}


char incomingPacket[255]; // Buffer para leer los paquetes UDP

bool hay_paquete_udp() {  //Prueba para detectar si hay paquetes junto comprobacion para descartar paquetes de clientes no autorizados.
  int packetSize = udp.parsePacket(); //revisa si llego un paquete, y si llego uno devuelve el tamaño.
  
  if (packetSize > 0 && clienteConectado && udp.remoteIP() != ipCliente) {
    // Si el paquete es de un cliente no autorizado, lo descartamos
    Serial.println("Paquete rechazado de cliente no autorizado: " + udp.remoteIP().toString());
    
    // Leer y descartar el paquete
    //udp.read(incomingPacket, sizeof(incomingPacket));
    udp.flush(); // O vaciar el buffer de paquetes, no se cual es mejor
    return false;  // Indicamos que no se debe procesar el paquete
  }
  return packetSize > 0;  // Si es un cliente autorizado, indicamos que hay paquete para procesar
}

bool leer_udp(){
  int len = udp.read(incomingPacket, 255);
  if (len > 0) {
    incomingPacket[len] = 0;  // Terminar el string
  }
  
  // Parsear el mensaje recibido (por ejemplo, "1,0,1")
  String packet = String(incomingPacket);
  int commaIndex = packet.indexOf(',');
  int sequenceNumber = packet.substring(0, commaIndex).toInt();  // Número de secuencia
  //String coordinates = packet.substring(commaIndex + 1);  // Coordenadas
  String datosPaquete = packet.substring(commaIndex + 1);   // String con el contenido del paquete sin el numero de secuencia.
  
  // Verificar si es un paquete más nuevo o si se reinicio el conteo de paquetes
  if ((sequenceNumber > lastSequence) || (lastSequence - sequenceNumber > (maxSeqNum / 2))) {
    lastSequence = sequenceNumber;  // Actualizar el último número de secuencia
    Serial.println("Paquete válido recibido: " + datosPaquete);
    procesar_paquete_udp(datosPaquete); //proceso el paquete
    return true;
  }
  else{
    Serial.println("Paquete descartado (llegó tarde o es duplicado)");
    return false;
  }
}

void procesar_paquete_udp(String datosPaquete) {
  datosPaquete.trim();  // Elimino posibles espacios en blanco, saltos y otros caracteres invisibles.
  
  int codigo = 0;  // Inicializamos el código para el switch.

  if (datosPaquete == "200") {
    codigo = 200;
  } else if (datosPaquete == "400") {
    codigo = 400;
  } else if (datosPaquete == "foto") {
    codigo = 1000;  // Asigno un valor arbitrario para "foto".
  }

  switch (codigo) {
    case 200:
      Serial.println("Me conecto a la CIAA.");
      //conexion_ciaa();
      break;
    
    case 400:
      Serial.println("Me desconecto de la CIAA.");
      desconectar_cliente();
      break;

    case 1000:
      alternar_modo();  // Paso a modo para tomar fotos
      tomar_foto();
      alternar_modo();  // Paso a modo streaming
      break;

    default:
      Serial.print("Envio a la CIAA: ");  // Debug
      Serial.println(datosPaquete);  // Envío a la ciaa si no coincide con "200", "400" o "foto".
      break;
  }
}

void conectar_cliente(){
  ipCliente = udp.remoteIP(); //Guardo la IP del cliente
  clienteConectado = true;
  Serial.println("Cliente conectado: " + ipCliente.toString());
  // Enviar la IP del ESP32 al cliente
  udp.beginPacket(ipCliente, udpPort);
  String mensaje = "AP IP:" + ipServidor.toString();  // Construir el mensaje como String
  Serial.print("Envio al cliente:");
  Serial.println(mensaje);
  udp.write(reinterpret_cast<uint8_t*>(&mensaje), sizeof(mensaje));       // Enviar el mensaje como uint8_t*
  udp.endPacket(); // Enviar respuesta
  ultimoMensaje = millis(); //Para que no entre en el if de timeout en el primer mensaje
}

void desconectar_cliente(){
  ipCliente = IPAddress(0, 0, 0, 0);  // Asignar la IP "vacía" 0.0.0.0;
  clienteConectado = false;
  lastSequence = 0; //Reinicio el numero de secuencia.
  //Serial.println("400");  //Deconecto la CIAA
  Serial.println("Desconectado de la CIAA.");
}