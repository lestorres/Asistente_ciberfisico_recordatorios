#include <Wire.h>
#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x27, 16, 2);

String ultimoMensaje = "";
String mensajeAnterior = "";
bool nuevoMensajeDisponible = false;
unsigned long tiempoUltimoMensaje = 0;
int ciclosFallidos = 0;

SemaphoreHandle_t mutexMensaje;

// Historial para detectar repetidos
#define HISTORIAL_MAX 15
String historialMensajes[HISTORIAL_MAX];
int indiceHistorial = 0;

bool contieneCaracteresInvalidos(const String &mensaje) {
  const char* invalidos = "áéíóúÁÉÍÓÚñÑ";
  for (int i = 0; i < mensaje.length(); i++) {
    if (strchr(invalidos, mensaje.charAt(i))) {
      return true;
    }
  }
  return false;
}

bool esMensajeRepetido(const String &mensaje) {
  for (int i = 0; i < HISTORIAL_MAX; i++) {
    if (historialMensajes[i] == mensaje) {
      return true;
    }
  }
  return false;
}

void agregarAMensajesRecientes(const String &mensaje) {
  historialMensajes[indiceHistorial] = mensaje;
  indiceHistorial = (indiceHistorial + 1) % HISTORIAL_MAX;
}

void TaskEnviarSolicitud(void *pvParameters) {
  for (;;) {
    Serial.println("Generar");
    vTaskDelay(30000 / portTICK_PERIOD_MS);
  }
}

void TaskEscucharSerial(void *pvParameters) {
  for (;;) {
    String recibido = "";
    unsigned long tiempoInicio = millis();
    while (millis() - tiempoInicio < 5000) {
      if (Serial.available()) {
        char c = Serial.read();
        if (c == '\n') break;
        recibido += c;
      }
      vTaskDelay(10 / portTICK_PERIOD_MS);
    }

    if (recibido.length() > 0) {
      xSemaphoreTake(mutexMensaje, portMAX_DELAY);

      // Reintento si es repetido o contiene caracteres inválidos
      if (esMensajeRepetido(recibido) || contieneCaracteresInvalidos(recibido)) {
        Serial.println("Generar");
        xSemaphoreGive(mutexMensaje);
        continue;
      }

      mensajeAnterior = ultimoMensaje;
      ultimoMensaje = recibido;
      nuevoMensajeDisponible = true;
      tiempoUltimoMensaje = millis();
      ciclosFallidos = 0;
      agregarAMensajesRecientes(ultimoMensaje);

      xSemaphoreGive(mutexMensaje);
    } else {
      ciclosFallidos++;
    }

    vTaskDelay(500 / portTICK_PERIOD_MS);
  }
}

void TaskLCDMostrar(void *pvParameters) {
  for (;;) {
    if (nuevoMensajeDisponible) {
      xSemaphoreTake(mutexMensaje, portMAX_DELAY);

      lcd.backlight();
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print(ultimoMensaje.substring(0, 16));
      lcd.setCursor(0, 1);
      if (ultimoMensaje.length() > 16) {
        lcd.print(ultimoMensaje.substring(16, 32));
      }

      nuevoMensajeDisponible = false;
      xSemaphoreGive(mutexMensaje);
    }
    vTaskDelay(200 / portTICK_PERIOD_MS);
  }
}

void TaskEstadoSerial(void *pvParameters) {
  for (;;) {
    if (!nuevoMensajeDisponible && ultimoMensaje == "") {
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Esperando...");
    }
    vTaskDelay(5000 / portTICK_PERIOD_MS);
  }
}

void TaskHeartbeat(void *pvParameters) {
  for (;;) {
    // No hace nada visible (sin consola)
    vTaskDelay(15000 / portTICK_PERIOD_MS);
  }
}

void TaskControlFlujo(void *pvParameters) {
  for (;;) {
    // Placeholder
    vTaskDelay(1000 / portTICK_PERIOD_MS);
  }
}

void TaskManejoErrores(void *pvParameters) {
  for (;;) {
    if (ciclosFallidos >= 8) {
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("ERROR: Fallos");
      lcd.setCursor(0, 1);
      lcd.print("Seguidos: " + String(ciclosFallidos));
      ciclosFallidos = 0;
    }
    vTaskDelay(10000 / portTICK_PERIOD_MS);
  }
}

void TaskPowerManager(void *pvParameters) {
  for (;;) {
    unsigned long ahora = millis();
    if (ahora - tiempoUltimoMensaje > 60000) {
      lcd.noBacklight();
    }
    vTaskDelay(10000 / portTICK_PERIOD_MS);
  }
}

void setup() {
  Serial.begin(115200);
  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("Iniciando RTOS...");
  delay(2000);
  lcd.clear();
  lcd.print("Hola!, soy tu");
  lcd.setCursor(0, 1);
  lcd.print("asistente RTOS");
  delay(2000);
  lcd.clear();

  mutexMensaje = xSemaphoreCreateMutex();

  xTaskCreatePinnedToCore(TaskEnviarSolicitud, "EnviarSolicitud", 2048, NULL, 1, NULL, 1);
  xTaskCreatePinnedToCore(TaskEscucharSerial, "EscucharSerial", 4096, NULL, 1, NULL, 1);
  xTaskCreatePinnedToCore(TaskLCDMostrar, "LCDMostrar", 2048, NULL, 1, NULL, 1);
  xTaskCreatePinnedToCore(TaskEstadoSerial, "EstadoLCD", 2048, NULL, 1, NULL, 1);
  xTaskCreatePinnedToCore(TaskHeartbeat, "Heartbeat", 2048, NULL, 1, NULL, 1);
  xTaskCreatePinnedToCore(TaskControlFlujo, "ControlFlujo", 2048, NULL, 1, NULL, 1);
  xTaskCreatePinnedToCore(TaskManejoErrores, "ManejoErrores", 2048, NULL, 1, NULL, 1);
  xTaskCreatePinnedToCore(TaskPowerManager, "PowerManager", 2048, NULL, 1, NULL, 1);
}

void loop() {
  // Todo se maneja con tareas
}
