#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>
#include "clientecpp.h"

class Robot {
private:
    int estadoIP; //0 para conectado, 1 para desconectado
    int estadoSerie; //0 para conectado, 1 para desconetado
    char modo; // "m" para manual, "a" para automatico
    Cliente* cliente;
public:
    / Robot() : estadoIP(1), estadoSerie(1), modo('m') {} //inicializamos con todo desconetado y en modo manual

    ~Robot() {
        if (cliente != nullptr) { delete cliente; } 
    }

    void conectarIP() { // Corrige el tipo a std::string
        if (estadoIP==0){
            std::cout<<"Ya esta conectado a la IP"<<std::endl;
        } else {
            if(cliente==nullptr){
                cliente = new Cliente();
            }
            estadoIP=cliente->conectarIP();
            if (estadoIP==0){
                std::cout<<"Se conectó a la IP exitosamente"<<std::endl;
            } else {
                std::cout<<"Se produjo un error al conectar la IP"<<std::endl;
            }
        }
    }

    void desconectarIP() {
        if (estadoIP==1){
            std::cout<<"Ya esta desconectado a la IP"<<std::endl;
        } else {
            if(cliente==nullptr){
                cliente = new Cliente();
            }
            cliente->desconectarIP();
            if (estadoIP==1){
                std::cout<<"Se desconectó a la IP exitosamente"<<std::endl;
            } else {
                std::cout<<"Se produjo un error al desconectar la IP"<<std::endl;
            }
        }
    }

    void conectarSerie() { // No entiendo que valores devuelve conectarSerie
            if (cliente == nullptr) {
                 cliente = new Cliente();
            }
            int estadoSerie = cliente->conectarSerie();
            if (estadoSerie== 0) {
                std::cout << "Conexión al puerto serie exitosa." << std::endl;
            } else {
                std::cout << "Fallo en la conexión al puerto serie." << std::endl;
        }
    }
    
    void desconectarSerie() { // Corrige el tipo a std::string
        if (estadoSerie==1){
            std::cout<<"Ya esta desconectado del pueto Serie"<<std::endl;
        } else {
            if(cliente==nullptr){
                cliente = new Cliente();
            }
            cliente->desconectarIP();
            if (estadoSerie==1){
                std::cout<<"Se desconectó del pueto Serie exitosamente"<<std::endl;
            } else {
                std::cout<<"Se produjo un error al desconectar del puerto Serie"<<std::endl;
            }
        }
    }


    void selecModo(char modoInput) {
        if (modoInput == 'a') {
            modo = true; // Cambiar a modo automático
            std::cout << "Se cambió a modo automático." << std::endl;
        } else if (modoInput == 'm') {
            modo = false; // Cambiar a modo manual
            std::cout << "Se cambió a modo manual." << std::endl;
        } else {
            std::cout << "Modo no válido." << std::endl;
        }
    }

    void encenderMotor() {
        std::cout << "Motor encendido." << std::endl;
    }

    void apagarMotor() {
        std::cout << "Motor apagado." << std::endl;
    }

    void home() {
        std::cout << "Homing completado." << std::endl;
    }

    void mostrarAyuda() {
        std::cout << "Ordenes disponibles:" << std::endl;
        std::cout << "help" << std::endl;
        std::cout << "man: Manual de uso" << std::endl;
        std::cout << "Conectar: conectar <IP>" << std::endl;
        std::cout << "Desconectar: desconectar" << std::endl;
        std::cout << "Encender motores: encenderMotor" << std::endl;
        std::cout << "Apagar motores: apagarMotor" << std::endl;
        std::cout << "Seleccionar modo: selecModo (a: automático, m: manual)" << std::endl;
        std::cout << "Mostrar reporte: reporteGeneral" << std::endl;
        std::cout << "Log de Trabajo del Servidor: reporteLog" << std::endl;
        std::cout << "--------------------------------------------------------------" << std::endl;
    }

    void mostrarMan() {
        std::cout << "selecModo <modo>" << std::endl;
        std::cout << "    Cambiar el modo de operación del robot entre automático o manual usando las letras a o m." << std::endl;
        std::cout << "encenderMotor" << std::endl;
        std::cout << "    Activar los motores del robot" << std::endl;
        std::cout << "apagarMotor" << std::endl;
        std::cout << "    Desactivar los motores del robot" << std::endl;
        std::cout << "reporteGeneral" << std::endl;
        std::cout << "    Mostrar reporte con:" << std::endl;
        std::cout << "    Conteniendo el estado de la conexión" << std::endl;
        std::cout << "    Posición y estado de actividad actual" << std::endl;
        std::cout << "    Momento en que se inició la actividad" << std::endl;
        std::cout << "    Listado con el detalle de las órdenes." << std::endl;
        std::cout << "reporteLog" << std::endl;
        std::cout << "    Mostrar el log de trabajo del servidor." << std::endl;
        std::cout << "--------------------------------------------------------------" << std::endl;
    }
};

int main() {
    Robot robot;
    std::string comando;

    // Simulación de inicio de sesión
    std::string usuario, contrasena;
    std::cout << "Ingrese usuario y contraseña:" << std::endl;
    std::cout << "Usuario: ";
    std::cin >> usuario;
    std::cout << "Contraseña: ";
    std::cin >> contrasena;

    std::cout << "Ingresado exitosamente" << std::endl;

    // Bucle principal de la interfaz
    while (true) {
        std::cout << ">> ";
        std::cin >> comando;

        if (comando == "man") {
            robot.mostrarMan();
        } else if (comando == "help") {
            robot.mostrarAyuda();
        } else if (comando == "conectar"){
            robot.conectar("192.168.1.1");
        } else if (comando == "desconectar"){
            robot.desconectar();
        }else if (comando == "selecModo") {
            char modo;
            std::cin >> modo;
            robot.selecModo(modo);
        } else if (comando == "encenderMotor") {
            robot.encenderMotor();
        } else if (comando == "apagarMotor") {
            robot.apagarMotor();
        } else if (comando == "home") {
            robot.home();
        } else if (comando == "exit") {
            std::cout << "Saliendo del programa." << std::endl;
            break;
        } else {
            std::cout << "Comando no reconocido. Escriba 'help' para ver los comandos disponibles." << std::endl;
        }
    }

    return 0;
}
