import cmd
import json
import os
from datetime import datetime
from typing import List, Dict, Optional
from Comunicacion_Controlador import ArduinoSerialController
from servidor1 import Servidor

class ConfigManager:
    """Clase para manejar la configuración del robot"""
    def __init__(self, config_file: str = "robot_config.json"):
        self.config_file = config_file
        self.default_config = {
            "puerto_serie": "COM3",
            "baudrate": 115200,
            "timeout": 1.0,
            "servidor_rpc": {
                "host": "localhost",
                "puerto": 8000,
                "activo": False
            }
        }
        self.load_config()

    def load_config(self) -> None:
        try:
            with open(self.config_file, 'r') as f:
                self.config = json.load(f)
        except (FileNotFoundError, json.JSONDecodeError):
            self.config = self.default_config
            self.save_config()

    def save_config(self) -> None:
        with open(self.config_file, 'w') as f:
            json.dump(self.config, f, indent=4)

class RobotController:
    """Clase para manejar la comunicación con el robot"""
    def __init__(self, config_manager: ConfigManager):
        self.config = config_manager
        self.serial_controller: Optional[ArduinoSerialController] = None
        self.motors_enabled = False
        self.work_mode = "manual"  # "manual" o "automatico"

    def connect(self) -> bool:
        try:
            self.serial_controller = ArduinoSerialController(
                port=self.config.config["puerto_serie"],
                baud_rate=self.config.config["baudrate"],
                timeout=self.config.config["timeout"]
            )
            self.serial_controller.connect()
            return True
        except Exception as e:
            print(f"Error al conectar: {e}")
            return False

    def disconnect(self) -> None:
        if self.serial_controller:
            self.serial_controller.disconnect()
            self.serial_controller = None

    def send_command(self, command: str) -> List[str]:
        if not self.serial_controller:
            print("No hay conexión establecida")
            return []
        
        responses = self.serial_controller.send_gcode(command)
        return responses if responses else []

    def get_status(self) -> Dict:
        return {
            "connected": self.serial_controller is not None,
            "motors_enabled": self.motors_enabled,
            "work_mode": self.work_mode,
            "port": self.config.config["puerto_serie"]
        }

class RobotCLI(cmd.Cmd):
    """Interfaz de línea de comandos para el control del robot"""
    intro = 'Bienvenido al panel de control del robot. Escriba help o ? para listar los comandos.\n'
    prompt = '(robot) '

    def __init__(self):
        super().__init__()
        self.config_manager = ConfigManager()
        self.robot_controller = RobotController(self.config_manager)
        self.is_admin = False
        self.trajectory_folder = "trayectorias"
        os.makedirs(self.trajectory_folder, exist_ok=True)

    def do_login(self, arg):
        """Login como administrador usando contraseña"""
        password = input("Ingrese la contraseña de administrador: ")
        # En una implementación real, usar hash y salt
        if password == "admin":
            self.is_admin = True
            print("Login exitoso como administrador")
        else:
            print("Contraseña incorrecta")

    def do_config(self, arg):
        """Mostrar/editar los parámetros de conexión del robot"""
        if not self.is_admin:
            print("Se requieren privilegios de administrador")
            return

        print("\nConfiguración actual:")
        print(json.dumps(self.config_manager.config, indent=4))
        
        if input("\n¿Desea editar la configuración? (s/n): ").lower() == 's':
            self.config_manager.config["puerto_serie"] = input("Puerto serie (ejemplo: COM3): ")
            self.config_manager.config["baudrate"] = int(input("Baudrate (default: 115200): "))
            self.config_manager.config["timeout"] = float(input("Timeout (default: 1.0): "))
            self.config_manager.save_config()
            print("Configuración actualizada")

    def do_rpc(self, arg):
        """Encender o apagar el servidor RPC"""
        if not self.is_admin:
            print("Se requieren privilegios de administrador")
            return

        if arg.lower() == "on":
            self.config_manager.config["servidor_rpc"]["activo"] = True
            self.servidor.start()
            print("Servidor RPC activado")
        elif arg.lower() == "off":
            self.config_manager.config["servidor_rpc"]["activo"] = False
            self.servidor.stop()
            print("Servidor RPC desactivado")
        else:
            print("Uso: rpc [on|off]")

    def do_log(self, arg):
        """Mostrar las últimas 100 líneas del log"""
        if not self.is_admin:
            print("Se requieren privilegios de administrador")
            return

        try:
            with open("server_log.json", 'r') as f: #aca cambiar el nombre al archivo del log del servidor
                logs = json.load(f)
                for log in logs[-100:]:
                    print(f"{log['timestamp']}: {log['message']}")
        except FileNotFoundError:
            print("Archivo de log no encontrado")

    def do_connect(self, arg):
        """Conectar con el robot"""
        if self.robot_controller.connect():
            print("Conexión establecida")
        else:
            print("Error al conectar")

    def do_disconnect(self, arg):
        """Desconectar del robot"""
        self.robot_controller.disconnect()
        print("Desconectado")

    def do_motors(self, arg):
        """Activar/desactivar motores (on/off)"""
        if arg.lower() not in ["on", "off"]:
            print("Uso: motors [on|off]")
            return

        command = "M17" if arg.lower() == "on" else "M84"
        responses = self.robot_controller.send_command(command)
        self.robot_controller.motors_enabled = (arg.lower() == "on")
        
        print(f"Motores {'activados' if self.robot_controller.motors_enabled else 'desactivados'}")
        if responses:
            print("Respuesta del robot:")
            for response in responses:
                print(f"  {response}")

    def do_status(self, arg):
        """Mostrar información general del robot"""
        status = self.robot_controller.get_status()
        print("\nEstado del robot:")
        for key, value in status.items():
            print(f"{key}: {value}")

    def do_mode(self, arg):
        """Cambiar modo de trabajo (manual/automatico)"""
        if arg.lower() not in ["manual", "automatico"]:
            print("Uso: mode [manual|automatico]")
            return

        self.robot_controller.work_mode = arg.lower()
        print(f"Modo de trabajo cambiado a: {arg}")

    def do_manual_commands(self, arg):
        """Listar comandos disponibles en modo manual"""
        if self.robot_controller.work_mode != "manual":
            print("Este comando solo está disponible en modo manual")
            return

        print("\nComandos disponibles:")
        print("1. M3 - Activar efector")
        print("2. M5 - Desactivar efector")
        print("3. G28 - Hacer homing")
        print("4. G1 Xa Yb Zc - Mover a posición (a,b,c)")

    def do_command(self, arg):
        """Enviar un comando G-code o M-code directamente al robot"""
        if not arg:
            print("Por favor, ingrese un comando")
            return

        responses = self.robot_controller.send_command(arg)
        if responses:
            print("Respuestas del robot:")
            for response in responses:
                print(f"  {response}")

    def do_move(self, arg):
        """Mover el robot a una posición específica"""
        if self.robot_controller.work_mode != "manual":
            print("Este comando solo está disponible en modo manual")
            return

        try:
            x = float(input("Coordenada X: "))
            y = float(input("Coordenada Y: "))
            z = float(input("Coordenada Z: "))
            command = f"G1 X{x} Y{y} Z{z}"
            responses = self.robot_controller.send_command(command)
            
            if responses:
                print("Respuestas del robot:")
                for response in responses:
                    print(f"  {response}")
        except ValueError:
            print("Por favor ingrese valores numéricos válidos")

    def do_save_trajectory(self, arg):
        """Crear un archivo de trayectoria"""
        if self.robot_controller.work_mode != "manual":
            print("Este comando solo está disponible en modo manual")
            return

        filename = input("Nombre del archivo de trayectoria: ")
        if not filename.endswith('.txt'):
            filename += '.txt'
        
        print("Ingrese comandos (línea vacía para terminar):")
        commands = []
        while True:
            command = input("> ")
            if not command:
                break
            commands.append(command)

        with open(os.path.join(self.trajectory_folder, filename), 'w') as f:
            f.write('\n'.join(commands))
        print(f"Trayectoria guardada en {filename}")

    def do_list_trajectories(self, arg):
        """Listar y ejecutar archivos de trayectoria disponibles"""
        if self.robot_controller.work_mode != "automatico":
            print("Este comando solo está disponible en modo automático")
            return

        files = os.listdir(self.trajectory_folder)
        if not files:
            print("No hay archivos de trayectoria disponibles")
            return

        print("\nArchivos de trayectoria disponibles:")
        for i, file in enumerate(files, 1):
            print(f"{i}. {file}")

        try:
            selection = int(input("\nSeleccione un archivo (número): ")) - 1
            if 0 <= selection < len(files):
                with open(os.path.join(self.trajectory_folder, files[selection]), 'r') as f:
                    commands = f.read().splitlines()
                    for command in commands:
                        print(f"\nEjecutando: {command}")
                        responses = self.robot_controller.send_command(command)
                        if responses:
                            print("Respuestas del robot:")
                            for response in responses:
                                print(f"  {response}")
            else:
                print("Selección inválida")
        except ValueError:
            print("Por favor ingrese un número válido")

    def do_exit(self, arg):
        """Salir del programa"""
        if self.robot_controller.serial_controller:
            self.robot_controller.disconnect()
        return True

if __name__ == '__main__':
    RobotCLI().cmdloop()
