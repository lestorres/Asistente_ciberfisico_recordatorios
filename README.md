# 🧠 Asistente_ciberfisico_recordatorios  
**Proyecto 3 - Taller de Sistemas Embebidos**  
Sistema de un Asistente ciberfísico para recordatorios a adultos mayores  

---

## 📖 Descripción

Este proyecto consiste en el desarrollo de un **asistente ciberfísico embebido** capaz de mostrar **mensajes positivos y recordatorios útiles** para personas adultas mayores. El sistema se basa en una arquitectura distribuida que combina un **ESP32 ejecutando FreeRTOS**, una **pantalla LCD con modulo I2C** y un modelo de lenguaje **LLaMA 3 ejecutado localmente en una PC** mediante **Ollama**.

El sistema es capaz de generar mensajes como:
- `Toma tu medicina a la 1pm`
- `Dios te cuida siempre`
- `Usa los lentes para leer`
- `Descansa un rato`

---

## ⚙️ Tecnologías utilizadas

- 🔌 **ESP32** (con FreeRTOS)
- 📟 **Pantalla LCD 16x2 I2C**
- 🧠 **LLaMA 3 (vía Ollama, local)**
- 🐍 **Python 3** (para comunicación serial y ejecución del modelo)
- 📡 **Comunicación Serial UART**
- 💻 **Arduino IDE / PlatformIO**

---

