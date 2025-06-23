import serial           # Comunicación serie con el ESP32
import subprocess       # Para ejecutar el modelo LLaMA 3 usando `ollama`
import time             # Para manejar tiempos de espera

# Configuración del puerto serial
SERIAL_PORT = 'COM7'      # Cambiar según el puerto utilizado en tu PC
BAUD_RATE = 115200        # Debe coincidir con la velocidad en el ESP32

# Prompt inicial que define el comportamiento del modelo LLaMA 3
PROMPT_INICIAL = """
Actua como un asistente virtual RTOS que corre en un ESP32.
Genera un mensaje corto (maximo 32 caracteres) para adultos mayores.
El mensaje puede ser un recordatorio util o una frase positiva.
No repitas los ultimos 5 mensajes (el sistema lo controla).
Por favor, genera mensajes sin acentos (tildes) ni la letra ñ.
Usa solo caracteres ASCII basicos.

Ejemplos:
- Eres amado
- Haz ejercicio
- Dios te cuida siempre
- Lee la biblia
- Ten paciencia
- Recuerda descansar
- Usa tus lentes
- Cierra bien la puerta
- Te amo mucho, mama
- Te amo mucho, papa

Cuando diga "Generar", responde con un solo mensaje. Sin explicaciones, solo el texto.

Esperando comando: "Generar"
"""

# Función para ejecutar el modelo LLaMA 3 con el prompt y obtener un mensaje
def generar_mensaje():
    print("[INFO] Generando nuevo mensaje con LLaMA 3...")
    try:
        # Ejecuta el modelo `llama3` usando Ollama
        proceso = subprocess.Popen(
            ["ollama", "run", "llama3"],
            stdin=subprocess.PIPE,
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
            encoding="utf-8",
            errors="ignore",
            text=True
        )

        # Escribe el prompt inicial y el comando "Generar"
        proceso.stdin.write(PROMPT_INICIAL + "\n")
        proceso.stdin.write("Generar\n")
        proceso.stdin.flush()

        # Espera la salida del modelo con un tiempo máximo de 15 segundos
        salida, _ = proceso.communicate(timeout=15)

        # Procesa la salida del modelo línea por línea
        for linea in salida.splitlines():
            linea = linea.strip()
            if linea:
                return linea  # Devuelve la primera línea válida

        return "[modelo sin respuesta]"

    except subprocess.TimeoutExpired:
        # Si el modelo no responde a tiempo, se termina el proceso
        proceso.kill()
        print("[ERROR] Timeout. El modelo no respondió.")
        return "[error: timeout]"

# Función principal: conexión con el ESP32 y manejo de solicitudes
def main():
    print(f"[INFO] Conectando a {SERIAL_PORT} a {BAUD_RATE} baudios...")
    ser = serial.Serial(SERIAL_PORT, BAUD_RATE, timeout=1)
    time.sleep(2)  # Da tiempo para que el puerto serie se estabilice

    print("[INFO] Escuchando al ESP32...\n")

    # Bucle principal: espera por solicitudes del ESP32
    while True:
        if ser.in_waiting:  # Si hay datos disponibles en el puerto
            linea = ser.readline().decode(errors='ignore').strip()  # Lee la línea
            if linea:
                print(f"[ESP32 -> PC] {linea}")

                if linea == "Generar":
                    # Si el ESP32 solicita un mensaje, lo genera con LLaMA 3
                    mensaje = generar_mensaje()

                    # Muestra el mensaje generado en consola
                    print(f"[LLaMA 3 -> PC] {mensaje}")

                    # Envía el mensaje al ESP32 por serial
                    ser.write((mensaje + "\n").encode())
                    print(f"[PC -> ESP32] {mensaje}\n")

# Punto de entrada del script
if __name__ == "__main__":
    main()
