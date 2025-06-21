#include <WiFi.h>  // Agrega esta línea para usar WiFi.macAddress()

void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println();

  Serial.print("Chip model: ");
  Serial.println(ESP.getChipModel());

  Serial.print("Chip revision: ");
  Serial.println(ESP.getChipRevision());

  Serial.print("CPU cores: ");
  Serial.println(getCpuCores());

  Serial.print("CPU frequency (MHz): ");
  Serial.println(ESP.getCpuFreqMHz());

  Serial.print("Flash size (bytes): ");
  Serial.println(ESP.getFlashChipSize());

  Serial.print("Flash speed (Hz): ");
  Serial.println(ESP.getFlashChipSpeed());

  Serial.print("Flash mode: ");
  printFlashMode(ESP.getFlashChipMode());

  Serial.print("Heap size (bytes): ");
  Serial.println(ESP.getHeapSize());

  Serial.print("Free heap (bytes): ");
  Serial.println(ESP.getFreeHeap());

  Serial.print("SDK version: ");
  Serial.println(ESP.getSdkVersion());

  Serial.print("MAC address: ");
  Serial.println(WiFi.macAddress());

  Serial.print("Temperature sensor (approx °C): ");
  Serial.println(readTemperature());

  Serial.println();
}

void loop() {
  // No hace nada
}

int getCpuCores() {
  return ESP.getChipCores();
}

void printFlashMode(uint8_t mode) {
  switch(mode) {
    case 0: Serial.println("QIO"); break;
    case 1: Serial.println("QOUT"); break;
    case 2: Serial.println("DIO"); break;
    case 3: Serial.println("DOUT"); break;
    default: Serial.println("Unknown"); break;
  }
}

float readTemperature() {
  return temperatureRead() / 100.0;
}
