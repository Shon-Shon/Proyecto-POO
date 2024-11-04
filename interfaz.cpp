#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>
#include <memory>
#include "clientecpp.h"

class Robot {
private:
    int estadoIP; //0 para conectado, 1 para desconectado
    int estadoSerie; //0 para conectado, 1 para desconectado
    char modo; // "m" para manual, "a" para automatico
    std::unique_ptr<Cliente> cliente;
public:
    // Constructor
    Robot() : estadoIP(1), estadoSerie(1), modo('m'),  cliente(std::make_unique<Cliente>()) {
    } //inicializamos con todo desconectado y en modo manual

    bool usuarioContrasenia(const std::string& user, const std::string& pass){
        cliente->guardarUsuario(user,pass);
        bool sec=cliente->verificarUsuario();
        return sec;
    }

    int conectarIP() { // Corrige el tipo a std::string
        if (estadoIP==0){
            std::cout<<"Ya esta conectado a la IP"<<std::endl;
        } else {
            estadoIP=cliente->conectarIP();
            if (estadoIP==0){
                std::cout<<"Se conectó a la IP exitosamente"<<std::endl;
            } else {
                std::cout<<"Se produjo un error al conectar la IP"<<std::endl;
            }
        }
        return estadoIP;
    }

    int desconectarIP() { // Corrige el tipo a std::string
        if (estadoIP==1){
            std::cout<<"Ya esta desconectado a la IP"<<std::endl;
        } else {
            cliente->desconectarIP();
            estadoIP=1;
            std::cout<<"Se desconectó de la IP"<<std::endl;
        }
        return estadoIP;
    }

    void conectarSerie() { 
            estadoSerie = cliente->conectarSerie();
            if (estadoSerie== 0) {
                std::cout << "Conexión al puerto serie exitosa" << std::endl;
            } else {
                std::cout << "Fallo en la conexión al puerto serie" << std::endl;
        }
    }
    
    void desconectarSerie() { // Corrige el tipo a std::string
        if (estadoSerie==1){
            std::cout<<"Ya esta desconectado del puerto Serie"<<std::endl;
        } else {
            cliente->desconectarIP();
            estadoSerie=1;
            std::cout<<"Se desconectó del puerto Serie"<<std::endl;
        }
    }

    void selecModo(char modoI) {
        if (modoI != 'a' && modoI != 'm') {
            std::cout << modoI << " no es un modo válido. Use 'a' para automático o 'm' para manual." << std::endl;
            return;
        }

        if (modoI == modo) {
            std::cout << "El modo ya está en " << (modo == 'a' ? "automático." : "manual.") << std::endl;
            return;
        }

        cliente->selecModo(modoI);
        modo = modoI;

        if (modoI == 'a') {
            std::cout << "Cambiado a modo automático." << std::endl;
            modoAutomatico();
        } else {
            std::cout << "Cambiado a modo manual." << std::endl;
            modoManual();
        }
    }

    void modoAutomatico() {
    std::string nombreArchivo;
    std::cout << "Ingrese el nombre del archivo para enviar: ";
    std::cin >> nombreArchivo;
    bool sobreescribir;
    std::cout << "¿Desea sobreescribir el archivo en el destino? (1 para sí, 0 para no): ";
    std::cin >> sobreescribir;
    int resultadoEnvio = cliente->enviarArchivo(nombreArchivo, sobreescribir);
    if (resultadoEnvio == 0) {
        std::cout << "Archivo enviado exitosamente." << std::endl;
        int resultadoEjecucion = cliente->ejecutarArchivo(nombreArchivo);
        if (resultadoEjecucion == 0) {
            std::cout << "Archivo ejecutado exitosamente." << std::endl;
        } else {
            std::cout << "Error al ejecutar el archivo." << std::endl;
        }
    } else {
        std::cout << "Error al enviar el archivo." << std::endl;
    }
}

    void modoManual() {
    double x, y, z, vel;
    char flag;
    while (true) {
        std::cout << "Ingrese x: ";
        std::cin >> x;
        std::cout << "Ingrese y: ";
        std::cin >> y;
        std::cout << "Ingrese z: ";
        std::cin >> z;

        std::cout << "¿Desea especificar velocidad? (Y/N): ";
        std::cin >> flag;
        if (flag == 'Y' || flag == 'y') {
            std::cout << "Ingrese velocidad: ";
            std::cin >> vel;
            cliente->moverXYZ(x, y, z, vel);
        } else if (flag == 'N' || flag == 'n') {
            cliente->moverXYZ(x, y, z);
        } else {
            std::cout << "Opción no válida, intente de nuevo." << std::endl;
            continue;
        }
        
        char continuar;
        std::cout << "¿Desea mover el robot a otra posición? (Y/N): ";
        std::cin >> continuar;
        if (continuar == 'N' || continuar == 'n') {
            break;
        }
    }
}


    std::string pedirLog(){
        return cliente->pedirLog();
    }
    std::string reporteGeneral(){
        return cliente->pedirRegistro();
    }

    void encenderMotor() {
        cliente->encenderMotor();
        std::cout<<"Motor encendido"<<std::endl;
    }

    void apagarMotor() {
        cliente->apagarMotor();
        std::cout << "Motor apagado." << std::endl;
    }

    void home() {
        cliente->home();
        std::cout << "Homing completado." << std::endl;
    }

    void mostrarAyuda() {
        std::cout << "Ordenes disponibles:" << std::endl;
        std::cout << "help" << std::endl;
        std::cout << "Manual de uso: man" << std::endl;
        std::cout << "Conectar a IP: conectarIP" << std::endl;
        std::cout << "Desonectar a IP: desconectarIP" << std::endl;
        std::cout << "Ingresar con Usuario y Contrasenia: logIn" << std::endl;
        std::cout << "Conectar puerto serie: conectarSerie" << std::endl;
        std::cout << "Desconectar de puerto serie: desconectarSerie" << std::endl;
        std::cout << "Encender motores: encenderMotor" << std::endl;
        std::cout << "Apagar motores: apagarMotor" << std::endl;
        std::cout << "Seleccionar modo: selecModo (a: automático, m: manual)" << std::endl;
        std::cout << "Mostrar reporte: reporteGeneral" << std::endl;
        std::cout << "Log de Trabajo del Servidor: pedirLog" << std::endl;
        std::cout << "Home: home" << std::endl;
        std::cout << "--------------------------------------------------------------" << std::endl;
    }

    void mostrarMan() {
        std::cout << "selecModo <modo>" << std::endl;
        std::cout << "    Cambiar el modo de operación del robot entre automático o manual usando las letras a o m. Por defecto se inicializa en modo manual." << std::endl;
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
    std::string comando, usuario,contrasena;
    bool sec=false;
    char modo;
    int estadoIP=1;


    while (true) {
        std::cout << ">>> ";
        std::cin >> comando;

        if (comando == "man") {
            robot.mostrarMan();
        } else if (comando == "help") {
            robot.mostrarAyuda();
        } else if (comando=="conectarIP") {
            estadoIP=robot.conectarIP();
        } else if (comando=="desconectarIP"){
            estadoIP=robot.desconectarIP();
        } else if (comando == "conectarSerie"){
            robot.conectarSerie();
        } else if (comando == "desconectarSerie"){
            robot.desconectarSerie(); 
        }else if(comando=="logIn"){
            if (estadoIP==0){
                while (!sec) {
                    std::cout << "Ingrese usuario: ";
                    std::cin >> usuario;
                    std::cout << "Ingrese contraseña: ";
                    std::cin >> contrasena;
                    sec = robot.usuarioContrasenia(usuario, contrasena);
                    if (!sec) {
                        std::cout << "Usuario o contraseña incorrectos. Intente nuevamente." << std::endl;
                    } else {
                        std::cout<<"Se ingresó de manera correcta"<<std::endl;
                    }
                } 
            } else{
                std::cout<<"Para logearse necesita estar conectado al servido IP"<<std::endl;
            }
        }else if (comando == "selecModo") {
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
        } else if (comando == "pedirLog") {
            std::cout << "Log del servidor:\n" << robot.pedirLog() << std::endl;
        } else if (comando == "reporteGeneral") {
            std::cout << "Reporte general:\n" << robot.reporteGeneral() << std::endl;
        } else {
            std::cout << "Comando no reconocido. Escriba 'help' para ver los comandos disponibles." << std::endl;
        }
    }

    return 0;
}
