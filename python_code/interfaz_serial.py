import serial
import subprocess
import time

# Configuración del puerto serial
SERIAL_PORT = 'COM7'  # Cambia si usas otro puerto
BAUD_RATE = 115200

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

def generar_mensaje():
    print("[INFO] Generando nuevo mensaje con LLaMA 3...")
    try:
        proceso = subprocess.Popen(
            ["ollama", "run", "llama3"],
            stdin=subprocess.PIPE,
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
            encoding="utf-8",
            errors="ignore",
            text=True
        )

        proceso.stdin.write(PROMPT_INICIAL + "\n")
        proceso.stdin.write("Generar\n")
        proceso.stdin.flush()

        salida, _ = proceso.communicate(timeout=15)
        for linea in salida.splitlines():
            linea = linea.strip()
            if linea:
                return linea
        return "[modelo sin respuesta]"
    except subprocess.TimeoutExpired:
        proceso.kill()
        print("[ERROR] Timeout. El modelo no respondió.")
        return "[error: timeout]"

def main():
    print(f"[INFO] Conectando a {SERIAL_PORT} a {BAUD_RATE} baudios...")
    ser = serial.Serial(SERIAL_PORT, BAUD_RATE, timeout=1)
    time.sleep(2)  # Espera para estabilizar conexión

    print("[INFO] Escuchando al ESP32...\n")

    while True:
        if ser.in_waiting:
            linea = ser.readline().decode(errors='ignore').strip()
            if linea:
                print(f"[ESP32 -> PC] {linea}")

                if linea == "Generar":
                    mensaje = generar_mensaje()

                    print(f"[LLaMA 3 -> PC] {mensaje}")
                    ser.write((mensaje + "\n").encode())
                    print(f"[PC -> ESP32] {mensaje}\n")

if __name__ == "__main__":
    main()
