#include <Wire.h>                   // Comunicación I2C
#include <LiquidCrystal_I2C.h>      // Librería para controlar pantallas LCD por I2C

// Inicializa el LCD con dirección I2C 0x27 y dimensiones 16x2
LiquidCrystal_I2C lcd(0x27, 16, 2);

// Variables globales para manejo de mensajes
String ultimoMensaje = "";
String mensajeAnterior = "";
bool nuevoMensajeDisponible = false;
unsigned long tiempoUltimoMensaje = 0;
int ciclosFallidos = 0;

// Mutex para proteger acceso a variables compartidas entre tareas
SemaphoreHandle_t mutexMensaje;

// Historial de mensajes para evitar duplicados
#define HISTORIAL_MAX 15
String historialMensajes[HISTORIAL_MAX];
int indiceHistorial = 0;

// Verifica si un mensaje contiene caracteres especiales no soportados por el LCD
bool contieneCaracteresInvalidos(const String &mensaje) {
  const char* invalidos = "áéíóúÁÉÍÓÚñÑ";
  for (int i = 0; i < mensaje.length(); i++) {
    if (strchr(invalidos, mensaje.charAt(i))) {
      return true;
    }
  }
  return false;
}

// Verifica si un mensaje ya fue mostrado anteriormente
bool esMensajeRepetido(const String &mensaje) {
  for (int i = 0; i < HISTORIAL_MAX; i++) {
    if (historialMensajes[i] == mensaje) {
      return true;
    }
  }
  return false;
}

// Agrega un mensaje al historial para evitar repeticiones
void agregarAMensajesRecientes(const String &mensaje) {
  historialMensajes[indiceHistorial] = mensaje;
  indiceHistorial = (indiceHistorial + 1) % HISTORIAL_MAX;
}

// Tarea: solicita un nuevo mensaje enviando "Generar" por el puerto serial cada 30 segundos
void TaskEnviarSolicitud(void *pvParameters) {
  for (;;) {
    Serial.println("Generar");
    vTaskDelay(30000 / portTICK_PERIOD_MS);
  }
}

// Tarea: escucha el puerto serial por nuevos mensajes durante 5 segundos
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

      // Ignora mensaje si es repetido o contiene caracteres inválidos
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

// Tarea: muestra el último mensaje recibido en la pantalla LCD
void TaskLCDMostrar(void *pvParameters) {
  for (;;) {
    if (nuevoMensajeDisponible) {
      xSemaphoreTake(mutexMensaje, portMAX_DELAY);

      lcd.backlight();
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print(ultimoMensaje.substring(0, 16));  // Primera línea
      lcd.setCursor(0, 1);
      if (ultimoMensaje.length() > 16) {
        lcd.print(ultimoMensaje.substring(16, 32));  // Segunda línea
      }

      nuevoMensajeDisponible = false;
      xSemaphoreGive(mutexMensaje);
    }
    vTaskDelay(200 / portTICK_PERIOD_MS);
  }
}

// Tarea: muestra mensaje "Esperando..." cuando no hay mensaje nuevo disponible
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

// Tarea: función "heartbeat" para mantener activo el sistema (placeholder)
void TaskHeartbeat(void *pvParameters) {
  for (;;) {
    vTaskDelay(15000 / portTICK_PERIOD_MS);
  }
}

// Tarea: control de flujo (aún sin implementación)
void TaskControlFlujo(void *pvParameters) {
  for (;;) {
    vTaskDelay(1000 / portTICK_PERIOD_MS);
  }
}

// Tarea: muestra mensaje de error en el LCD si se acumulan fallos seguidos
void TaskManejoErrores(void *pvParameters) {
  for (;;) {
    if (ciclosFallidos >= 8) {
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("ERROR: Fallos");
      lcd.setCursor(0, 1);
      lcd.print("Seguidos: " + String(ciclosFallidos));
      ciclosFallidos = 0;  // Reset después de mostrar el error
    }
    vTaskDelay(10000 / portTICK_PERIOD_MS);
  }
}

// Tarea: apaga la luz de fondo del LCD si ha pasado más de 60 segundos sin nuevo mensaje
void TaskPowerManager(void *pvParameters) {
  for (;;) {
    unsigned long ahora = millis();
    if (ahora - tiempoUltimoMensaje > 60000) {
      lcd.noBacklight();
    }
    vTaskDelay(10000 / portTICK_PERIOD_MS);
  }
}

// Función principal: inicialización del sistema
void setup() {
  Serial.begin(115200);       // Inicializa comunicación serial
  lcd.init();                 // Inicializa el LCD
  lcd.backlight();            // Enciende luz de fondo
  lcd.setCursor(0, 0);
  lcd.print("Iniciando RTOS...");
  delay(2000);
  lcd.clear();
  lcd.print("Hola!, soy tu");
  lcd.setCursor(0, 1);
  lcd.print("asistente RTOS");
  delay(2000);
  lcd.clear();

  mutexMensaje = xSemaphoreCreateMutex();  // Crea el mutex

  // Crea todas las tareas en el núcleo 1 del ESP32
  xTaskCreatePinnedToCore(TaskEnviarSolicitud, "EnviarSolicitud", 2048, NULL, 1, NULL, 1);
  xTaskCreatePinnedToCore(TaskEscucharSerial, "EscucharSerial", 4096, NULL, 1, NULL, 1);
  xTaskCreatePinnedToCore(TaskLCDMostrar, "LCDMostrar", 2048, NULL, 1, NULL, 1);
  xTaskCreatePinnedToCore(TaskEstadoSerial, "EstadoLCD", 2048, NULL, 1, NULL, 1);
  xTaskCreatePinnedToCore(TaskHeartbeat, "Heartbeat", 2048, NULL, 1, NULL, 1);
  xTaskCreatePinnedToCore(TaskControlFlujo, "ControlFlujo", 2048, NULL, 1, NULL, 1);
  xTaskCreatePinnedToCore(TaskManejoErrores, "ManejoErrores", 2048, NULL, 1, NULL, 1);
  xTaskCreatePinnedToCore(TaskPowerManager, "PowerManager", 2048, NULL, 1, NULL, 1);
}

// Bucle principal: no se usa, todo está en tareas
void loop() {
  // Las tareas de FreeRTOS se encargan del flujo
}
