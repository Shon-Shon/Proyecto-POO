import serial
import time

class ArduinoSerialController:
    def __init__(self, port, baud_rate=115200, timeout=1):
        self.port = port
        self.baud_rate = baud_rate
        self.timeout = timeout
        self.connection = None

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
        """Envía una orden en g-code al Arduino y espera la respuesta."""
        if self.connection and self.connection.is_open:
            try:
                # Agrega \r\n al final del comando
                command = f"{gcode_command}\r\n"
                print("Enviando comando:", command.strip())
                
                # Limpia el buffer antes de enviar
                self.connection.reset_input_buffer()
                self.connection.reset_output_buffer()
                
                # Envía el comando
                self.connection.write(command.encode('utf-8'))
                self.connection.flush()
                
                # Espera un momento para dar tiempo a Arduino de procesar
                time.sleep(0.1)
                
                # Lee todas las líneas de respuesta disponibles
                responses = []
                while self.connection.in_waiting:
                    try:
                        response = self.connection.readline().decode('utf-8').strip()
                        if response:
                            responses.append(response)
                            print("Respuesta:", response)
                    except UnicodeDecodeError as e:
                        print("Error decodificando respuesta:", e)
                
                return responses
                
            except Exception as e:
                print(f"Error en la comunicación: {str(e)}")
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
                    responses.append(response)
            except UnicodeDecodeError:
                continue
        return responses
    
    @classmethod
    def comando_actGripper(clc):
        return "M3"
    
    @classmethod
    def comando_deactGripper(clc):
        retutn "M5"
    
    @classmethod
    def comando_home(clc):
        return "G28"
    
    @classmethod
    def comando_moverXYZ(clc, x,y,z,v):
        return f"G1 X{x} Y{y} Z{z} F{v}"
    
    @classmethod
    def comando_coordAbs(clc):
        return "G90"
    
    @classmethod
    def comando_coordRel(clc):
        return "G91"
    
    @classmethod
    def comando_encenderMotor(clc):
        return "M17"
    
    @classmethod
    def comando_apagarMotor(clc):
        return "M18"

if __name__ == "__main__":
    controller = ArduinoSerialController(port='COM6')  # Cambia 'COM6' al puerto adecuado
    controller.connect()

    try:
        while True:
            user_command = input("Introduce el comando G-code o M-code (o escribe 'exit' para salir): ")
            if user_command.lower() == 'exit':
                break
            
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
        print("\nPrograma interrumpido por el usuario")
    finally:
        controller.disconnect()
