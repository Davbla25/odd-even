import pygame
import sys
import math
import array

# --- CONFIGURACIÓN ---
TRACE_FILE = "trace.txt"
WIDTH, HEIGHT = 1000, 600
FPS = 60  # Velocidad de la animación (bájalo si va muy rápido)

# Colores (RGB)
BG_COLOR = (20, 20, 30)       # Fondo oscuro
BAR_NORMAL = (100, 200, 255)  # Azul claro
BAR_CMP = (255, 255, 0)       # Amarillo (Comparando)
BAR_SWP = (255, 50, 50)       # Rojo (Intercambiando)
BAR_DONE = (50, 255, 100)     # Verde (Ordenado)

# --- INICIALIZAR PYGAME Y AUDIO ---
# pre_init para asegurar que el sonido no tenga retraso (44.1kHz, 16 bits, estéreo)
pygame.mixer.pre_init(44100, -16, 2, 512)
pygame.init()
screen = pygame.display.set_mode((WIDTH, HEIGHT))
pygame.display.set_caption("Benchmark x86 - Odd-Even Sort Visualizer")
clock = pygame.time.Clock()

# Usaremos un canal dedicado para que los pitidos no se solapen creando ruido
audio_channel = pygame.mixer.Channel(0)

# --- FUNCIÓN GENERADORA DE SONIDO (Sintetizador 8-bit) ---
def play_sound_for_value(value, max_val):
    """ Genera un pitido cuya frecuencia depende del valor (altura) de la barra """
    # Mapeamos el valor del array a una frecuencia entre 200 Hz (grave) y 1200 Hz (agudo)
    min_freq, max_freq = 200, 1200
    freq = min_freq + (value / max_val) * (max_freq - min_freq)
    
    duration = 0.05  # Duración del pitido en segundos (50 ms)
    sample_rate = 44100
    n_samples = int(sample_rate * duration)
    
    # Creamos el buffer de audio (Array de enteros cortos de 16 bits)
    buf = array.array('h')
    for i in range(n_samples):
        t = float(i) / sample_rate
        # Onda senoidal (volumen al 15% para no dejarte sordo)
        v = int(0.15 * math.sin(2 * math.pi * freq * t) * 32767)
        buf.append(v)
        buf.append(v) # Canal derecho (Estéreo)
    
    sound = pygame.mixer.Sound(buf)
    audio_channel.play(sound)

# --- PARSEAR EL ARCHIVO TRACE.TXT ---
def load_trace(filename):
    n = 0
    arr = []
    actions =[]
    
    try:
        with open(filename, 'r') as f:
            for line in f:
                parts = line.strip().split()
                if not parts: continue
                
                cmd = parts[0]
                if cmd == "INIT":
                    n = int(parts[1])
                elif cmd == "VALS":
                    arr =[int(x) for x in parts[1:]]
                elif cmd in ("CMP", "SWP"):
                    actions.append((cmd, int(parts[1]), int(parts[2])))
    except FileNotFoundError:
        print(f"ERROR: No se encuentra '{filename}'. Ejecuta el programa en C primero.")
        sys.exit(1)
        
    return n, arr, max(arr), actions

# --- BUCLE PRINCIPAL DE LA ANIMACIÓN ---
def main():
    n, arr, max_val, actions = load_trace(TRACE_FILE)
    
    # Cálculo del ancho de las barras para que ocupen toda la pantalla
    bar_width = WIDTH / n
    
    action_idx = 0
    running = True
    finished = False
    finish_sweep_idx = 0  # Para la animación verde del final

    while running:
        clock.tick(FPS) # Controlar la velocidad de fotogramas
        
        # Procesar eventos (como darle a la X para cerrar)
        for event in pygame.event.get():
            if event.type == pygame.QUIT:
                running = False

        # --- Lógica de la animación ---
        hl_idx1, hl_idx2 = -1, -1 # Índices resaltados
        current_cmd = ""
        
        if not finished:
            if action_idx < len(actions):
                cmd, i, j = actions[action_idx]
                current_cmd = cmd
                hl_idx1, hl_idx2 = i, j
                
                # Reproducimos sonido basado en la barra que estamos mirando
                play_sound_for_value(arr[i], max_val)
                
                if cmd == "SWP":
                    # Intercambiamos visualmente los valores en el array de Python
                    arr[i], arr[j] = arr[j], arr[i]
                
                action_idx += 1
            else:
                finished = True
        
        # --- Dibujar en pantalla ---
        screen.fill(BG_COLOR)
        
        for i in range(n):
            # Altura proporcional a la pantalla
            bar_height = (arr[i] / max_val) * (HEIGHT - 50)
            x = i * bar_width
            y = HEIGHT - bar_height
            
            # Decidir color de la barra
            color = BAR_NORMAL
            
            if not finished:
                if i == hl_idx1 or i == hl_idx2:
                    color = BAR_CMP if current_cmd == "CMP" else BAR_SWP
            else:
                # Animación barrido verde al terminar
                if i < finish_sweep_idx:
                    color = BAR_DONE
                    
            rect = pygame.Rect(x, y, max(1, bar_width - 1), bar_height)
            pygame.draw.rect(screen, color, rect)
            
        # Si hemos terminado, avanzar el barrido verde y hacer un sonido de victoria
        if finished and finish_sweep_idx <= n:
            if finish_sweep_idx < n:
                play_sound_for_value(arr[finish_sweep_idx], max_val)
            finish_sweep_idx += 2
            
        pygame.display.flip()

    pygame.quit()
    sys.exit()

if __name__ == "__main__":
    main()