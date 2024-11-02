import socket
import json
from jsonrpc import JSONRPCResponseManager, Dispatcher
import random
import time
import functools
import mmap
import re


def unpack_dict(func):
    @functools.wraps(func)
    def wrapper(*args, **kwargs):
        #revisamos el último parámetro posicional para ver si es un diccionario
        if args and isinstance(args[-1], dict):
            kwargs.update(args[-1])  #actualizamos kwargs con el diccionario
            return func(*args[:-1], **kwargs)    #llamamos a la función sin el diccionario final en args
        return func(*args, **kwargs)  #llamamos a la función normalmente
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


class Validador_usuarios:
    def __init__(self, archivo="usuarios.csv"):
        self.archivo_usuarios = archivo
    
    def validar_usuario(self, user):
        print("llegamos al validador de usuarios")
        with open(self.archivo_usuarios, 'rb', 0) as file, \
            mmap.mmap(file.fileno(), 0, access=mmap.ACCESS_READ) as s:
            posicion = re.search(br'^' + user.usuario.encode('utf-8') + br';[^\n]*?\n',s)
            if posicion != None:
                print("usuario encontrado")
                s.seek(posicion.start()+len(user.usuario)+1)
                cont = s.readline().strip()
                print(cont)
                if cont.decode('utf-8') == user.contrasenia:
                    return True
            return False

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

"""
def deserializar(serializado):
    if not serializado.__class__ is dict: raise TypeError("Solo se puede serializar diccionarios")
    if (not "clase" in serializado) or (not "atributos" in serializado):
        raise ValueError("Falta atributo \"clase\" con nombre de clase o atributo \"atributos\" con conjunto de los atributos")
    if not serializado["clase"].__class__ is str: raise TypeError("\"clase\" debe ser string")
    if not serializado["clase"].__class__ is str: raise TypeError("\"atributos\" debe ser string")
    if any([(not x.__class__ is str) for x in serializado["atributos"]]): raise TypeError("nombre de atributos debe ser string")
    #
    return globals()[serializado["clase"]].deserializar(serializado["atributos"])

def deserializar_entrada(func):
    @functools.wraps(func)
    def wrapper_deserializar_entrada(*args, **kwargs):
        deser_args = []
        for i in args:
            if i.__class__ is dict and "clase" in i:
                print("objeto")
                deser_args.append(deserializar(i))
            else: deser_args.append(i)
        deser_kwargs = dict()
        for k,v in kwargs.items():
            if v is dict and "clase" in v:
                print("objeto")
                deser_kwargs.update({k:deserializar(v)})
            else: deser_kwargs.update({k:v})
        return func(*deser_args,**deser_kwargs)
    return wrapper_deserializar_entrada
"""

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
        print("llegamos al deserializador de Usuario")
        print("Usuario",usuario," Cont",contrasenia)
        return clc(usuario, contrasenia)


Clases = {"Usuario":Usuario, "Lectura":Lectura}

def deserializar(diccionario):
    print("llegamos a la función deserializar")
    
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

validador = Validador_usuarios()

despachador.add_method( serializar_salida(unpack_dict( Lectura.medir )), name="medir" )
despachador.add_method( unpack_dict( printear ) )
despachador.add_method( deserializar_entrada(validador.validar_usuario), name="validar_usuario")

# Configuración del servidor
HOST = '127.0.0.1'
PORT = 5000

def handle_request(data):
    # Procesar la solicitud JSON-RPC
    response = JSONRPCResponseManager.handle(data, despachador)
    return response.data

# Crear el socket del servidor
with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
    s.bind((HOST, PORT))
    s.listen()

    print(f'Servidor JSON-RPC escuchando en {HOST}:{PORT}')


    conn, addr = s.accept()
    with conn:
        print(f'Conectado a {addr}')
        while True:
            data = conn.recv(1024).decode('utf-8')
            if not data:
                break

            # Manejar la solicitud y enviar respuesta
            response = handle_request(data)
            conn.sendall(json.dumps(response).encode('utf-8'))
