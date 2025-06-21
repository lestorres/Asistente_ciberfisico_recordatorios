import subprocess
import time

prompt1 = """
Eres un asistente virtual RTOS que corre en un ESP32, conectado a una pantalla LCD 16x2.
Tu funcion es ayudar a personas adultas mayores, generando mensajes breves, positivos o utiles.
Cada mensaje debe tener un maximo de 32 caracteres, ideal para mostrarse en dos lineas de 16 caracteres.

Requisitos:
- El mensaje puede ser un recordatorio util o una frase positiva.
- No repitas los ultimos 2 mensajes.
- Solo responde con el texto del mensaje. Sin comillas, explicaciones, ni formato adicional.
- Si el mensaje tiene menos de 32 caracteres, esta bien. Solo no debe superar ese limite.

Cuando recibas el comando "Generar", responde con un mensaje segun lo indicado.

Ejemplos:
- Usa tus lentes para leer
- Dios te cuida, siempre contigo
- Descansa un poco, te lo mereces
- Cierra bien la puerta por favor
- Te amo mucho mama
- Hoy es un buen dia para ti
- Llama si necesitas ayuda
- No contestes numeros privados
- Cierra bien el porton al salir
- Haz tus ejercicios
- Usa los lentes al leer
- Te amo mama regreso pronto
- Te amo papa, cuidate mucho
- Numero emergencias en el refri
- Lee un libro y relajate
- Descansa si estas cansado
- Lee la Biblia, Dios te ama
- Apaga luces si no las usas
- Sonrie tu alegria ilumina
- Abre las ventanas y respira
- Llama si necesitas ayuda
- Disfruta este dia tranquilo
- Tu paz es lo mas valioso

Esperando comando: "Generar"
"""



prompt2 = "Generar"

print("[INFO] Iniciando modelo LLaMA 3 con prompt base...")

process = subprocess.Popen(
    ["ollama", "run", "llama3"],
    stdin=subprocess.PIPE,
    stdout=subprocess.PIPE,
    stderr=subprocess.PIPE,
    encoding="utf-8",
    errors="ignore",
    text=True
)

process.stdin.write(prompt1 + "\n")
process.stdin.flush()

print("[OK] Prompt inicial enviado.")
print("[WAIT] Esperando 3 segundos antes de generar mensaje...")
time.sleep(1)

print("[SEND] Enviando comando 'Generar' al modelo...")

try:
    stdout, stderr = process.communicate(input=prompt2 + "\n", timeout=10)
except subprocess.TimeoutExpired:
    process.kill()
    stdout, stderr = process.communicate()

response = ""
for line in stdout.splitlines():
    line = line.strip()
    if line:
        response = line
        break

print(f"[RESULTADO] Mensaje generado por LLaMA 3: {response}")
