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
    // Constructor
    Robot() : estadoIP(1), estadoSerie(1), modo('m') {
        cliente= new Cliente();
    } //inicializamos con todo desconetado y en modo manual

    ~Robot() {
        if (cliente != nullptr) { delete cliente; } 
    }

    bool usuarioContrasenia(const std::string& user, const std::string& pass){
        cliente->guardarUsuario(user,pass);
        bool sec=cliente->verificarUsuario();
        return sec;
    }

    void conectarIP() { // Corrige el tipo a std::string
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
    }

    void desconectarIP() { // Corrige el tipo a std::string
        if (estadoIP==1){
            std::cout<<"Ya esta desconectado a la IP"<<std::endl;
        } else {
            cliente->desconectarIP();
            estadoIP=1;
            std::cout<<"Se desconectó de la IP"<<std::endl;
        }
    }

    void conectarSerie() { // No entiendo que valores devuelve conectarSerie
            estadoSerie = cliente->conectarSerie();
            if (estadoSerie== 0) {
                std::cout << "Conexión al puerto serie exitosa" << std::endl;
            } else {
                std::cout << "Fallo en la conexión al puerto serie" << std::endl;
        }
    }
    
    void desconectarSerie() { // Corrige el tipo a std::string
        if (estadoSerie==1){
            std::cout<<"Ya esta desconectado del pueto Serie"<<std::endl;
        } else {
            cliente->desconectarIP();
            estadoSerie=1;
            std::cout<<"Se desconectó del puerto Serie"<<std::endl;
        }
    }

    void selecModo(char modoI) {
        bool modovalido = false;
        while (!modovalido) {
            if (modoI == modo) {
                if (modoI == 'm') {
                    modovalido = true;
                    std::cout << "Ya se encuentra en modo manual." << std::endl;
                } else if (modoI == 'a') {
                    modovalido = true;
                    std::cout << "Ya se encuentra en modo automático." << std::endl;
                }
            } else if (modoI == 'a') {
                modovalido = true;
                cliente->selecModo(modoI);
                modo = modoI;
                std::cout << "Se cambió a modo automático." << std::endl;
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
            } else if (modoI == 'm') {
                modovalido = true;
                bool m = false;
                double x, y, z, vel;
                char flag;
                cliente->selecModo(modoI);
                modo = modoI;
                std::cout << "Se cambió a modo manual." << std::endl;
                std::cout << "Ingrese la posición final que debe tomar el robot y la velocidad (opcional)." << std::endl;
                while (!m) {
                    std::cout << "Ingrese x: ";
                    std::cin >> x;
                    std::cout << "Ingrese y: ";
                    std::cin >> y;
                    std::cout << "Ingrese z: ";
                    std::cin >> z;
                    bool flagValida = false;
                    while (!flagValida) {
                        std::cout << "¿Desea especificar velocidad? Y/N: ";
                        std::cin >> flag;
                        if (flag == 'Y' || flag == 'y') {
                            std::cout << "Ingrese velocidad: ";
                            std::cin >> vel;
                            cliente->moverXYZ(x, y, z, vel);
                            flagValida = true;
                        } else if (flag == 'N' || flag == 'n') {
                            cliente->moverXYZ(x, y, z);
                            flagValida = true;
                        } else {
                            std::cout << "La opción no es válida. Intente nuevamente." << std::endl;
                        }
                    }
                } 
            } else { 
                std::cout << modoI << " no es un modo válido." << std::endl; 
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
        std::cout<<"Motod encendido"<<std::endl;
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
        std::cout << "man: Manual de uso" << std::endl;
        std::cout << "Conectar a IP: conectarIP" << std::endl;
        std::cout << "Conectar puerto serie: conectarSerie" << std::endl;
        std::cout << "Desconectar de puerto serie: desconectarSerie" << std::endl;
        std::cout << "Encender motores: encenderMotor" << std::endl;
        std::cout << "Apagar motores: apagarMotor" << std::endl;
        std::cout << "Seleccionar modo: selecModo (a: automático, m: manual)" << std::endl;
        std::cout << "Mostrar reporte: reporteGeneral" << std::endl;
        std::cout << "Log de Trabajo del Servidor: pedirLog" << std::endl;
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
    std::string comando;
    bool sec=false;
    std::string usuario,contrasena;
    char modo;

    while (!sec) {
        std::cout << "Ingrese usuario: ";
        std::cin >> usuario;
        std::cout << "Ingrese contraseña: ";
        std::cin >> contrasena;
        sec = robot.usuarioContrasenia(usuario, contrasena);
        if (!sec) { std::cout << "Usuario o contraseña incorrectos. Intente nuevamente." << std::endl;
        } else {
            std::cout<<"Se ingresó de manera correcta"<<std::endl;
        }
    }
    // Simulación de inicio de sesión
    

    // Bucle principal de la interfaz
    while (true) {
        std::cout << ">> ";
        std::cin >> comando;

        if (comando == "man") {
            robot.mostrarMan();
        } else if (comando == "help") {
            robot.mostrarAyuda();
        } else if (comando == "conectarSerie"){
            robot.conectarIP();
        } else if (comando == "desconectarSerie"){
            robot.desconectarSerie();
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
