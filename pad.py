import time
import pygame
import serial
import serial.tools.list_ports

BAUDRATE = 115200

# Cambia este puerto por el de tu Arduino.
# Ejemplos:
# Windows: COM5
# Linux: /dev/ttyACM0
PUERTO_ARDUINO = "COM3"

ZONA_MUERTA = 0.12
VELOCIDAD_MAXIMA = 255


def aplicar_zona_muerta(valor: float) -> float:
    """Elimina pequeños movimientos involuntarios del joystick."""
    if abs(valor) < ZONA_MUERTA:
        return 0.0

    # Reescala el valor después de quitar la zona muerta
    signo = 1 if valor > 0 else -1
    valor_ajustado = (abs(valor) - ZONA_MUERTA) / (1.0 - ZONA_MUERTA)

    return signo * valor_ajustado


def limitar(valor: int, minimo: int, maximo: int) -> int:
    return max(minimo, min(maximo, valor))


def main():
    pygame.init()
    pygame.joystick.init()

    if pygame.joystick.get_count() == 0:
        raise RuntimeError(
            "No se detectó ningún control. "
            "Conecta el Xbox Elite Series 2 por USB o Bluetooth."
        )

    control = pygame.joystick.Joystick(0)
    control.init()

    print(f"Control detectado: {control.get_name()}")
    print(f"Ejes disponibles: {control.get_numaxes()}")

    arduino = serial.Serial(
        port=PUERTO_ARDUINO,
        baudrate=BAUDRATE,
        timeout=0.1
    )

    # Arduino se reinicia al abrir el puerto serial
    time.sleep(2)

    print("Control iniciado.")
    print("Stick izquierdo: avanzar, retroceder y girar.")
    print("Presiona Ctrl+C para terminar.")

    try:
        while True:
            pygame.event.pump()

            # Normalmente:
            # Eje 0 = stick izquierdo horizontal
            # Eje 1 = stick izquierdo vertical
            eje_x = aplicar_zona_muerta(control.get_axis(0))
            eje_y = aplicar_zona_muerta(control.get_axis(1))

            # En pygame, hacia arriba suele ser negativo
            avance = int(-eje_y * VELOCIDAD_MAXIMA)
            giro = int(eje_x * VELOCIDAD_MAXIMA)

            avance = limitar(avance, -255, 255)
            giro = limitar(giro, -255, 255)

            mensaje = f"{avance},{giro}\n"
            arduino.write(mensaje.encode("utf-8"))

            print(
                f"\rAvance: {avance:4d} | Giro: {giro:4d}",
                end=""
            )

            time.sleep(0.03)

    except KeyboardInterrupt:
        print("\nDeteniendo carrito...")

    finally:
        try:
            arduino.write(b"0,0\n")
            time.sleep(0.1)
        finally:
            arduino.close()
            pygame.quit()


if __name__ == "__main__":
    try:
        main()
    except serial.SerialException as error:
        print(f"Error de comunicación con Arduino: {error}")
    except Exception as error:
        print(f"Error: {error}")