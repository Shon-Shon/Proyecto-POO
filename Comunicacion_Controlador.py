import serial 
import time

class ArduinoSerialController:
    def __init__(self, port, baud_rate=115200, timeout=1):
        # Inicializa el controlador con el puerto especificado, la tasa de baudios y el tiempo de espera
        self.port = port
        self.baud_rate = baud_rate
        self.timeout = timeout
        self.connection = None  # Atributo para almacenar la conexión serial

    def connect(self):
        """Establece la conexión con el Arduino."""
        try:
            # Configura la conexión serial con los parámetros especificados
            self.connection = serial.Serial(
                port=self.port,
                baudrate=self.baud_rate,
                timeout=self.timeout,
                write_timeout=1,
                bytesize=serial.EIGHTBITS,
                parity=serial.PARITY_NONE,
                stopbits=serial.STOPBITS_ONE
            )
            time.sleep(2)  # Espera para que Arduino inicialice
            print("Conexión establecida con el Arduino en", self.port)
        except serial.SerialException as e:
            print("Error al conectar:", e)  # Captura e informa errores de conexión

    def disconnect(self):
        """Cierra la conexión serie."""
        if self.connection and self.connection.is_open:
            self.connection.close()  # Cierra la conexión si está abierta
            print("Conexión cerrada")

    def send_gcode(self, gcode_command):
        """Envía una orden en g-code al Arduino y espera la respuesta."""
        if self.connection and self.connection.is_open:
            try:
                # Agrega \r\n al final del comando para enviar como línea completa
                command = f"{gcode_command}\r\n"
                print("Enviando comando:", command.strip())
                
                # Limpia el buffer antes de enviar para evitar interferencias
                self.connection.reset_input_buffer()
                self.connection.reset_output_buffer()
                
                # Envía el comando codificado en UTF-8 y espera un momento
                self.connection.write(command.encode('utf-8'))
                self.connection.flush()
                time.sleep(0.1)
                
                # Lee todas las respuestas disponibles en el buffer
                responses = []
                while self.connection.in_waiting:
                    try:
                        response = self.connection.readline().decode('utf-8').strip()
                        if response:  # Si hay una respuesta válida
                            responses.append(response)
                            print("Respuesta:", response)
                    except UnicodeDecodeError as e:
                        print("Error decodificando respuesta:", e)
                
                return responses  # Devuelve todas las respuestas acumuladas
                
            except Exception as e:
                print(f"Error en la comunicación: {str(e)}")  # Captura errores de envío/recepción
                return None
        else:
            print("Conexión no establecida")
            return None

    def read_all(self):
        """Lee todas las líneas disponibles en el buffer."""
        responses = []
        while self.connection.in_waiting:
            try:
                response = self.connection.readline().decode('utf-8').strip()
                if response:
                    responses.append(response)  # Agrega cada línea leída a la lista
            except UnicodeDecodeError:
                continue  # Ignora errores de decodificación
        return responses

    # Métodos de clase para generar comandos G-code específicos para el robot
    @classmethod
    def comando_actGripper(clc):
        return "M3"  # Activa la herramienta de agarre

    @classmethod
    def comando_deactGripper(clc):
        return "M5"  # Desactiva la herramienta de agarre

    @classmethod
    def comando_home(clc):
        return "G28"  # Lleva el robot a la posición de inicio

    @classmethod
    def comando_moverXYZ(clc, x, y, z, v):
        return f"G1 X{x} Y{y} Z{z} F{v}"  # Comando para mover el robot a coordenadas específicas con velocidad

    @classmethod
    def comando_coordAbs(clc):
        return "G90"  # Cambia a coordenadas absolutas

    @classmethod
    def comando_coordRel(clc):
        return "G91"  # Cambia a coordenadas relativas

    @classmethod
    def comando_encenderMotor(clc):
        return "M17"  # Enciende el motor

    @classmethod
    def comando_apagarMotor(clc):
        return "M18"  # Apaga el motor

# Bloque principal para pruebas manuales
if __name__ == "__main__":
    # Crea una instancia del controlador con el puerto especificado
    controller = ArduinoSerialController(port='COM6')  # Cambia 'COM6' al puerto adecuado para tu configuración
    controller.connect()  # Establece la conexión con el Arduino

    try:
        while True:
            # Solicita al usuario un comando G-code para enviar al Arduino
            user_command = input("Introduce el comando G-code o M-code (o escribe 'exit' para salir): ")
            if user_command.lower() == 'exit':
                break  # Termina el bucle si el usuario escribe 'exit'
            
            # Envía el comando ingresado al Arduino y muestra las respuestas recibidas
            responses = controller.send_gcode(user_command)
            if responses:
                print("Respuestas recibidas:")
                for response in responses:
                    print(f"  {response}")
            else:
                print("No se recibió respuesta")
                
            # Espera un momento antes de continuar
            time.sleep(0.1)

    except KeyboardInterrupt:
        print("\nPrograma interrumpido por el usuario")  # Maneja una interrupción por teclado (Ctrl+C)
    finally:
        controller.disconnect()  # Cierra la conexión al finalizar el programa
