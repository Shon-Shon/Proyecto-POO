import socket
import json
from jsonrpc import JSONRPCResponseManager, Dispatcher
import random
import time
import functools
import mmap
import re
import os
import threading
#Este hay que sacarlo después
import inspect
import Comunicacion_Controlador
from collections import deque
from datetime import datetime

#ArduinoSerialController
DEBUG=True

log=[]
registro=[]

class Log:
    def __init__(self, archivo_conexion="log_conexion.txt", archivo_general="log_general.txt"):
        self.archivo_conexion = archivo_conexion
        self.archivo_general = archivo_general
        self.log_conexion = deque()
        self.log_general = deque()
    
    def logeo_general(self, elemento, write_time = True):
        if write_time:
            self.log_general.append(datetime.now().strftime("%Y-%m-%d %H:%M:%S.%f"))
        self.log_general.append(elemento)
    
    def logeo_conexion(self, elemento, write_time = True):
        if write_time:
            self.log_conexion.append(datetime.now().strftime("%Y-%m-%d %H:%M:%S.%f"))
        self.log_conexion.append(elemento)
    
    def get_log_general(self):
        return iter(self.log_general)
    
    def get_log_conexion(self):
        return iter(self.log_conexion)
    
    def guardar_log_general(self):
        with open(self.archivo_general,"a") as archivo:
            for i in self.log_general:
                archivo.write(str(i)+"\n")
    
    def guardar_log_conexion(self):
        with open(self.archivo_conexion,"a") as archivo:
            for i in self.log_conexion:
                archivo.write(str(i)+"\n")

log = Log()

def unpack_dict(func):
    @functools.wraps(func)
    def wrapper(*args, **kwargs):
        #revisamos el último parámetro posicional para ver si es un diccionario
        if args and isinstance(args[-1], dict):
            kwargs.update(args[-1])  #actualizamos kwargs con el diccionario
            return func(*args[:-1], **kwargs)    #llamamos a la funcion sin el diccionario final en args
        return func(*args, **kwargs)  #llamamos a la funcion normalmente
    return wrapper


despachador = Dispatcher()

# Definir métodos JSON-RPC
@despachador.add_method
@unpack_dict
def add(x, y):
    return x + y

@despachador.add_method
@unpack_dict
def subtract(x, y):
    return x - y


def es_builtin(obj): return isinstance(obj, (int, float, str, list, dict, set, tuple, bool, bytes))

def serializar_salida(func):
    @functools.wraps(func)
    def wrapper_serializado(*args,**kwargs):
        valor = func(*args, **kwargs)
        if hasattr(valor,"__iter__"):
            return [a.serializado() if not es_builtin(a) else a for a in valor]
        return valor if es_builtin(valor) else valor.serializar()
        #ser_kwargs = dict((k,v.serializado()) for k,v in kwargs.items())
    return wrapper_serializado


class Lectura:
    def __init__(self, numero, tiempo):
            self.numero=numero
            self.tiempo=tiempo
    #
    @classmethod
    def medir(cls,b=0,t=1024):
            if b< -0x7fffffff - 1 or t>0x7fffffff: raise ValueError("parametros fuera de rango")
            if b>t: raise ValueError("el inicio del rango debe ser menor o igual al final del rango")
            num=random.randint(b,t)
            tim=time.localtime()
            return cls(num,tim)
    #
    def serializar(self):
         return {"clase":self.__class__.__name__ , "atributos":{"tiempo":time.asctime(self.tiempo) , "numero":self.numero} }
    
    @classmethod
    def deserializar(clc, numero, tiempo):
        return clc(numero,tiempo)


class Usuario:
    def __init__(self, usuario, contrasenia):
        self.usuario = usuario
        self.contrasenia = contrasenia
    
    @classmethod
    def deserializar(clc, usuario, contrasenia):
        return clc(usuario, contrasenia)


Clases = {"Usuario":Usuario, "Lectura":Lectura}

def deserializar(diccionario):
    
    if diccionario.__class__ is not dict:
        raise TypeError("El objeto para deserializar no es un diccionario")
    if "clase" not in diccionario:
        raise KeyError("El diccionario para deserializar no posee entrada 'clase'")
    if diccionario["clase"] not in Clases:
        raise NameError("La clase no es una clase conocida")
    else: clase = Clases[diccionario["clase"]]
    
    atributos=dict()
    if "atributos" in diccionario:
        atributos.update(diccionario["atributos"])
    
    return clase.deserializar(**atributos)

def deserializar_entrada(func):
    @functools.wraps(func)
    def wrapper(*args, **kwargs):
        args=list(args)
        for i,v in enumerate(args):
            if (v.__class__ is dict) and ("clase" in v):
                args[i] = deserializar(v)
        for k,v in kwargs:
            if (v.__class__ is dict) and ("clase" in v):
                kwargs[k] = deserializar(v)
        return func(*args,**kwargs)
    return wrapper


def printear(*args):
    salida=""
    for i in args:
        salida+=i.__str__()+" "
    print(salida)
    return salida


class Gestor_controlador:
    def __init__(self, puertoSerie="COM6"):
        if DEBUG: print("Se ejecuto",inspect.currentframe().f_code.co_name)
        self.controlador = Comunicacion_Controlador.ArduinoSerialController(puertoSerie)
        self.modo = "m"
        self.archivo = None
        self.nombre_archivo = ''
        self.log = []
        self.registro = []
    
    def selecModo(self, modo_deseado):
        if DEBUG: print("Se ejecuto",inspect.currentframe().f_code.co_name)
        log.logeo_general(f"Se ejecuto {inspect.currentframe().f_code.co_name}({modo_deseado})")
        self.modo = modo_deseado
    
    def conectarSerie(self):
        if DEBUG: print("Se ejecuto",inspect.currentframe().f_code.co_name)
        log.logeo_general(f"Se ejecuto {inspect.currentframe().f_code.co_name}")
        if not self.controlador.connection:
            self.controlador.connect()
        if not self.controlador.connection:
            log.logeo_general("Error: 1",write_time=False)
            return 1
        return 0
    
    def desconectarSerie(self):
        if DEBUG: print("Se ejecuto",inspect.currentframe().f_code.co_name)
        log.logeo_general(f"Se ejecuto {inspect.currentframe().f_code.co_name}")
        self.controlador.disconnect()
        self.controlador.connection = None
    
    def encenderMotor(self):
        if DEBUG: print("Se ejecuto",inspect.currentframe().f_code.co_name)
        log.logeo_general(f"Se ejecuto {inspect.currentframe().f_code.co_name}")
        #print("¡Brum Brum!")
        orden = self.controlador.comando_encenderMotor()
        respuesta = self.controlador.send_gcode(orden)
        
    def apagarMotor(self):
        if DEBUG: print("Se ejecuto",inspect.currentframe().f_code.co_name)
        log.logeo_general(f"Se ejecuto {inspect.currentframe().f_code.co_name}")
        #print("PIIUuuu..")
        orden = self.controlador.comando_apagarMotor()
        respuesta = self.controlador.send_gcode(orden)
    
    def moverXYZ(self, x,y,z,v=-1):
        if DEBUG: print("Se ejecuto",inspect.currentframe().f_code.co_name)
        log.logeo_general(f"Se ejecuto {inspect.currentframe().f_code.co_name}({x},{y},{z},{v})")
        if v==-1: v=2
        orden = self.controlador.comando_moverXYZ(x,y,z,v)
        respuesta = self.controlador.send_gcode(orden)
        return "".join(respuesta)
    
    def home(self):
        if DEBUG: print("Se ejecuto",inspect.currentframe().f_code.co_name)
        log.logeo_general(f"Se ejecuto {inspect.currentframe().f_code.co_name}")
        orden = self.controlador.comando_home()
        respuesta = self.controlador.send_gcode(orden)
    
    def actGripper(self):
        if DEBUG: print("Se ejecuto",inspect.currentframe().f_code.co_name)
        log.logeo_general(f"Se ejecuto {inspect.currentframe().f_code.co_name}")
        orden = self.controlador.comando_actGripper()
        respuesta = self.controlador.send_gcode(orden)
        
    def deactGripper(self):
        if DEBUG: print("Se ejecuto",inspect.currentframe().f_code.co_name)
        log.logeo_general(f"Se ejecuto {inspect.currentframe().f_code.co_name}")
        orden = self.controlador.comando_deactGripper()
        respuesta = self.controlador.send_gcode(orden)
    
    def recibirArchivo(self, nombre_archivo, sobreescribir=False):
        if DEBUG: print("Se ejecuto",inspect.currentframe().f_code.co_name)
        log.logeo_general(f"Se ejecuto {inspect.currentframe().f_code.co_name}({nombre_archivo},{sobreescribir})")
        if self.archivo != None:
            log.logeo_general("Error: 2",write_time=False)
            return 2
        if os.path.splitext(nombre_archivo)[1] != ".gcode":
            log.logeo_general("Error: 3",write_time=False)
            return 3
        direccion = os.path.join("G_Codes",nombre_archivo)
        if os.path.isfile(direccion) and sobreescribir==False:
            log.logeo_general("Warning: 1",write_time=False)
            return 1
        self.nombreArchivo = nombre_archivo
        self.archivo = open(direccion, 'wb')
        return 0
    
    def seguir_recibiendo_archivo(self, fragmento):
        log.logeo_general(f"Se ejecuto {inspect.currentframe().f_code.co_name}([fragmento de archivo])")
        if DEBUG: print("Se ejecuto",inspect.currentframe().f_code.co_name)
        fragmento_bin = fragmento.encode()
        #Acá hay que verificar que el archivo exista y esté abierto
        self.archivo.write(fragmento_bin)
        
    def terminar_recibir_archivo(self):
        if DEBUG: print("Se ejecuto",inspect.currentframe().f_code.co_name)
        log.logeo_general(f"Se ejecuto {inspect.currentframe().f_code.co_name}")
        if self.archivo:
            self.archivo.close()
        self.archivo = None
    
    def ejecutarArchivo(self, nombre_archivo):
        if DEBUG: print("Se ejecuto",inspect.currentframe().f_code.co_name)
        log.logeo_general(f"Se ejecuto {inspect.currentframe().f_code.co_name}({nombre_archivo})")
        if "." not in nombre_archivo: nombre_archivo += ".gcode"
        direccion = os.path.join("G_Codes",nombre_archivo)
        if not os.path.isfile(direccion):
            log.logeo_general("Error: 1",write_time=False)
            return 1
        with open(direccion, "r") as archivo:
            orden = archivo.readline().strip()
            resultado = "\n".join(self.controlador.send_gcode(orden))
            #resultado = "Lalala"
            if "ERROR" in resultado:
                log.logeo_general("Error: 2",write_time=False)
                return 2
        return 0
    
    def pedirLog(self):
        if DEBUG: print("Se ejecuto",inspect.currentframe().f_code.co_name)
        log.logeo_general(f"Se ejecuto{inspect.currentframe().f_code.co_name}")
        return "\n".join(log.get_log_conexion())
    
    def pedirRegistro(self):
        if DEBUG: print("Se ejecuto",inspect.currentframe().f_code.co_name)
        salida = "\n".join(log.get_log_general())
        log.logeo_general(f"Se ejecuto{inspect.currentframe().f_code.co_name}")
        return salida
    
    def registrar_funciones(self, despachador):
        if DEBUG: print("Se ejecuto",inspect.currentframe().f_code.co_name)
        despachador.add_method(self.selecModo)
        despachador.add_method(self.conectarSerie)
        despachador.add_method(self.desconectarSerie)
        despachador.add_method(self.encenderMotor)
        despachador.add_method(self.apagarMotor)
        despachador.add_method(self.moverXYZ)
        despachador.add_method(self.home)
        despachador.add_method(self.actGripper)
        despachador.add_method(self.deactGripper)
        despachador.add_method(unpack_dict(self.recibirArchivo), name="enviarArchivo")
        despachador.add_method(self.seguir_recibiendo_archivo, name="seguir_enviando")
        despachador.add_method(self.terminar_recibir_archivo, name="terminar_de_enviar")
        despachador.add_method(self.ejecutarArchivo)
        despachador.add_method(self.pedirLog)
        despachador.add_method(self.pedirRegistro)

class Servidor:
    def __init__(self, stop_event, gestor_cont, HOST = '127.0.0.1', PORT = 5000, archivo_usuarios="usuarios.csv"):
        # Inicializamos la clase con un evento de parada
        self.stop_event = stop_event
        self.server_thread = None
        self.gestor_cont = gestor_cont
        self.despachador_verificado = Dispatcher()
        self.despachador_no_verificado = Dispatcher()
        self.HOST = HOST
        self.PORT = PORT
        self.archivo_usuarios = archivo_usuarios
        #self.validador = Validador_usuarios()
        self.user_verified = False
        
        self.gestor_cont.registrar_funciones(self.despachador_verificado)
        self.despachador_verificado.add_method(deserializar_entrada(self.validar_usuario))
        self.despachador_no_verificado.add_method(deserializar_entrada(self.validar_usuario))
    
    def run(self):
        with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
            s.bind((self.HOST, self.PORT))
            s.listen()
            print(f'Servidor JSON-RPC escuchando en {self.HOST}:{self.PORT}')
            conn, addr = s.accept()
            with conn:
                print(f'Conectado a {addr}')
                log.logeo_conexion(f'Conectado a {addr}')
                while not self.stop_event.is_set():
                    data = conn.recv(2048).decode('utf-8')
                    if not data:
                        break
                    
                    # Manejar la solicitud y enviar respuesta
                    response = self.handle_request(data)
                    if DEBUG:
                        print("------------")
                        print(data)
                        print(response)
                    if response:
                        conn.sendall(json.dumps(response).encode('utf-8'))
                print("Servidor detenido.")
    
    def start(self):
        # Iniciamos el servidor en un hilo separado
        self.server_thread = threading.Thread(target=self.run)
        self.server_thread.start()
    
    def stop(self):
        # Detenemos el servidor
        self.stop_event.set()
        self.server_thread.join()
    
    def handle_request(self, request_str):
        #response = JSONRPCResponseManager.handle(request_str, despachador)
        if self.user_verified:
            response = JSONRPCResponseManager.handle(request_str, self.despachador_verificado)
            #self.user_verif = False
        else:
            response = response = JSONRPCResponseManager.handle(request_str, self.despachador_no_verificado)
        if response:
            return response.data
        return None
    
    def validar_usuario(self, user):
        if DEBUG: print("Se ejecuto",inspect.currentframe().f_code.co_name)
        with open(self.archivo_usuarios, 'rb', 0) as file, \
            mmap.mmap(file.fileno(), 0, access=mmap.ACCESS_READ) as s:
            posicion = re.search(br'^' + user.usuario.encode('utf-8') + br';[^\n]*?\n',s)
            if posicion != None:
                s.seek(posicion.start()+len(user.usuario)+1)
                cont = s.readline().strip()
                if cont.decode('utf-8') == user.contrasenia:
                    log.logeo_conexion(f"Accedio el usuario {user.usuario}")
                    self.user_verified = True
                    return True
            log.logeo_conexion(f"Se intento acceder con la cuenta de {user.usuario}")
            return False

class AdminInterface:
    def __init__(self, stop_event):
        self.stop_event = stop_event

    def start(self):
        while not self.stop_event.is_set():
            command = input("Escribe 'stop' para detener el servidor: ")
            if command.lower() == "stop":
                self.stop_event.set()
                print("Comando de parada recibido. Deteniendo el servidor...")


despachador.add_method( serializar_salida(unpack_dict( Lectura.medir )), name="medir" )
despachador.add_method( unpack_dict( printear ) )

gestor_cont = Gestor_controlador("COM6")

stop_event = threading.Event()
server = Servidor(stop_event, gestor_cont)
admin_interface = AdminInterface(stop_event)

server.start()
admin_interface.start()

server.stop()
log.guardar_log_conexion()
log.guardar_log_general()
print("El servidor ha sido detenido completamente.")
