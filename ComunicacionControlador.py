import serial
import time

class ArduinoSerialController:
    def __init__(self, port, baud_rate=115200, timeout=1):
        self.port = port
        self.baud_rate = baud_rate
        self.timeout = timeout
        self.connection = None
        self.commands_sent = []  # Lista para almacenar los comandos enviados
        self.responses_received = []  # Lista para almacenar las respuestas recibidas

    def connect(self):
        """Establece la conexión con el Arduino."""
        try:
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
            print("Error al conectar:", e)

    def disconnect(self):
        """Cierra la conexión serie."""
        if self.connection and self.connection.is_open:
            self.connection.close()
            print("Conexión cerrada")

    def send_gcode(self, gcode_command):
        """Envía una orden en g-code al Arduino y almacena las respuestas."""
        if self.connection and self.connection.is_open:
            try:
                # Agrega \r\n al final del comando
                command = f"{gcode_command}\r\n"
                print("Enviando comando:", command.strip())
                
                # Guarda el comando en la lista de comandos enviados
                self.commands_sent.append(command.strip())
                
                # Limpia el buffer antes de enviar
                self.connection.reset_input_buffer()
                self.connection.reset_output_buffer()
                
                # Envía el comando
                self.connection.write(command.encode('utf-8'))
                self.connection.flush()
                
                # Espera un momento para dar tiempo a Arduino de procesar
                time.sleep(0.1)
                
                # Lee todas las líneas de respuesta disponibles y las almacena
                while self.connection.in_waiting:
                    try:
                        response = self.connection.readline().decode('utf-8').strip()
                        if response:
                            self.responses_received.append(response)
                    except UnicodeDecodeError as e:
                        print("Error decodificando respuesta:", e)
                
            except Exception as e:
                print(f"Error en la comunicación: {str(e)}")

    def show_history(self):
        """Muestra el historial de comandos y respuestas recibidas."""
        print("\nHistorial de comandos enviados:")
        for command in self.commands_sent:
            print(f"  {command}")
        
        print("\nHistorial de respuestas recibidas:")
        for response in self.responses_received:
            print(f"  {response}")

if __name__ == "__main__":
    controller = ArduinoSerialController(port='COM6')  # Cambia 'COM6' al puerto adecuado
    controller.connect()

    try:
        while True:
            user_command = input("Introduce el comando G-code o M-code (o escribe 'exit' para salir): ")
            if user_command.lower() == 'exit':
                break
            
            controller.send_gcode(user_command)  # Envía el comando sin mostrar respuestas
            
            # Espera un momento antes de continuar
            time.sleep(0.1)

    except KeyboardInterrupt:
        print("\nPrograma interrumpido por el usuario")
    finally:
        controller.disconnect()
        controller.show_history()  # Muestra el historial al final
