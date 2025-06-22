# 🧠 Asistente ciberfisico para recordatorios  
**Proyecto 3 - Taller de Sistemas Embebidos**  
Sistema de un Asistente ciberfísico para recordatorios a adultos mayores  

---

## 📖 Descripción 

Este proyecto consiste en el desarrollo de un **asistente ciberfísico embebido** capaz de mostrar **mensajes positivos y recordatorios útiles** para personas adultas mayores. El sistema se basa en una arquitectura distribuida que combina un **ESP32 ejecutando FreeRTOS**, una **pantalla LCD con modulo I2C** y un modelo de lenguaje **LLaMA 3 ejecutado localmente en una PC** mediante **Ollama**.

El sistema es capaz de generar mensajes como:
- `Eres amado`
- `Haz ejercicio`
- `Dios te cuida siempre`
- `Lee la biblia`
- `Ten paciencia`

---

## ⚙️ Herramientas utilizadas

- 🔌 **ESP32** (con FreeRTOS)
- 📟 **Pantalla LCD 16x2 I2C**
- 🧠 **LLaMA 3 (vía Ollama, local)**
- 🐍 **Python 3** (para comunicación serial y ejecución del modelo)
- 📡 **Comunicación Serial UART**
- 💻 **Arduino IDE**

---

## Sistema FreeRTOS 

En la implementación de sistema con FreeRTOS en el ESP32, se debe orquestar el comportamiento del sistema por medio de Tareas (Tasks), cada una de ellas  tendrán prioridades y funcionalidades distintas, como se muestra en la Tabla 1.

### Tabla 1: Tareas FreeRTOS en el ESP32

| Tarea | Nombre               | Descripción                                                                 |
|-------|----------------------|-----------------------------------------------------------------------------|
| T1    | TaskEnviarSolicitud  | Genera y envía una solicitud cada 30 segundos para iniciar comunicación.   |
| T2    | TaskEscucharSerial   | Escucha el UART, recibe mensajes y valida su contenido.                    |
| T3    | TaskLCDMostrar       | Muestra mensajes nuevos en la pantalla LCD 16x2.                           |
| T4    | TaskEstadoSerial     | Muestra el estado “Esperando...” cuando no hay mensajes disponibles.       |
| T5    | TaskHeartbeat        | Indicador de vida: genera una señal/parpadeo cada 15 segundos.             |
| T6    | TaskControlFlujo     | Tarea reservada orquestar la ejecución definiendo el tiempo de ejecución.  |
| T7    | TaskManejoErrores    | Detecta errores UART consecutivos y muestra advertencias.                  |
| T8    | TaskPowerManager     | Apaga el backlight del LCD tras 60 segundos sin nuevos mensajes.           |


De manera visual se muestra el Diagrama de flujo del sistema a continuación. 

### Diagrama de Flujo 

```mermaid
---
config:
  theme: default
  layout: elk
---

flowchart TD
  subgraph subGraph0["Tareas Principales"]
    T1@{ label: "(T1) TaskEnviarSolicitud<br>Envia 'Generar' cada 30s" }
    T2["(T2) TaskEscucharSerial<br>Lee UART, filtra, almacena"]
    T3["(T3) TaskLCDMostrar<br>Muestra nuevo mensaje"]
    T4@{ label: "(T4) TaskEstadoSerial<br>Muestra 'Esperando...' si está vacío" }
    T5["(T7) TaskManejoErrores<br>Detecta ≥8 fallos"]
    T6["(T8) TaskPowerManager<br>Apaga luz tras 60s"]
  end

  A["Inicio del sistema<br>setup"] --> B["Inicializa LCD y UART"]
  B --> C["Crear mutexMensaje"]
  C --> D["Crear tareas RTOS"]
  D --> T1 & T2 & T3 & T4 & T5 & T6

  T1 -- UART: 'Generar' --> T2
  T2 -- Mensaje válido --> T3
  T2 -- Mensaje inválido o repetido --> T1
  T2 -- Contador de fallos --> T5
  T3 -- Actualizar LCD --> T6
  T4 -- "LCD: 'Esperando...'" --> T3
  T5 -- LCD: 'Error fallos' --> T3
  T6 -- No actividad 60s --> lcd["Apaga Backlight"]

  T1@{ shape: rect }
  T4@{ shape: rect }

```
Es importante aclarar que las tareas `(T5) TaskHeartbeat` y la tarea `(T6) TaskControlFlujo`, son tareas que por su naturaleza están implicitas dentro del sistema, estas se ejecutan cada cierto tiempo y definen aspectos importantes en la trama, razón por la que no aparecen en el diagrama de flujo de manera explícita.  



## Referencias

[1] Observatorio del Envejecimiento, Universidad de Costa Rica (UCR), *Primer Informe: Personas Adultas Mayores en Costa Rica*, 2023. [En línea]. Disponible en: [Primer Informe: Personas Adultas Mayores en Costa Rica ](https://fmedicina.ucr.ac.cr/wp-content/uploads/2025/06/1_PRIMER-INFORME_Envejecimiento.pdf)

[2] Instituto Nacional de Estadística y Censos (INEC), *Panorama Demográfico 2023*, San José, Costa Rica: INEC, 2023. Disponible en: [Panorama Demográfico 2023](https://admin.inec.cr/sites/default/files/2024-12/repoblacEV-Panorama%20demogr%C3%A1fico-2023A.pdf)

[3] Universidad Cenfotec, *Guía para conectar ESP32 a un servidor local con Ollama*, 2024. [En línea]. Disponible en: [Guía para conectar ESP32 a un servidor local con Ollama](https://github.com/Universidad-Cenfotec/ExpoCenfo/blob/main/Ejemplos_LLM_ESP32/ESP32_Ollama_Guide.md)



