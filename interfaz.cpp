#include <iostream>
#include <string>
#include <sstream>
#include <unordered_map>
#include <vector>
#include <memory>
#include <limits>
#include "clientecpp.h"

#define DEBUG

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
    
    int getEstadoIP(){return estadoIP;}
    int getEstadoSerie(){return estadoSerie;}
    
    //guarda el usuario, lo verifica y devuelve verdadero o falso según el resultado
    bool usuarioContrasenia(const std::string& user, const std::string& pass){
        cliente->guardarUsuario(user,pass);
        bool sec=cliente->verificarUsuario();
        return sec;
    }
    
    //conecta al servidor
    int conectarIP() { // Corrige el tipo a std::string
        if (estadoIP==0){
            std::cout<<"Ya esta conectado a la IP"<<std::endl;
        } else {
            estadoIP=cliente->conectarIP();
            if (estadoIP==0){
                std::cout<<"Se conecto a la IP exitosamente"<<std::endl;
            } else {
                std::cout<<"Se produjo un error al conectar la IP"<<std::endl;
            }
        }
        return estadoIP;
    }

    //desconecta del servidor
    int desconectarIP() { // Corrige el tipo a std::string
        if (estadoIP==1){
            std::cout<<"Ya esta desconectado a la IP"<<std::endl;
        } else {
            cliente->desconectarIP();
            estadoIP=1;
            std::cout<<"Se desconecto de la IP"<<std::endl;
        }
        return estadoIP;
    }

    //conecta el puerto serie en el servidor
    void conectarSerie() { 
            estadoSerie = cliente->conectarSerie();
            if (estadoSerie== 0) {
                std::cout << "Conexion al puerto serie exitosa" << std::endl;
            } else {
                std::cout << "Fallo en la conexion al puerto serie" << std::endl;
        }
    }
    
    //desconecta el puerto serie en el servidor
    void desconectarSerie() { // Corrige el tipo a std::string
        if (estadoSerie==1){
            std::cout<<"Ya esta desconectado del puerto Serie"<<std::endl;
        } else {
            cliente->desconectarIP();
            estadoSerie=1;
            std::cout<<"Se desconecto del puerto Serie"<<std::endl;
        }
    }

    //cambia el modo de operación entre manual y automático. Aquí el modo automático y manual son funciones
    void selecModo(char modoI) {
        if (modoI != 'a' && modoI != 'm') {
            std::cout << modoI << " no es un modo valido. Use 'a' para automatico o 'm' para manual." << std::endl;
            return;
        }
        
        //no permite usar el mismo modo dos veces, creo que hay que corregir eso.
        // if (modoI == modo) {
            // std::cout << "El modo ya esta en " << (modo == 'a' ? "automatico." : "manual.") << std::endl;
            // return;
        // }

        cliente->selecModo(modoI);
        modo = modoI;

        if (modoI == 'a') {
            std::cout << "Cambiado a modo automatico." << std::endl;
            modoAutomatico();
        } else {
            std::cout << "Cambiado a modo manual." << std::endl;
            modoManual();
        }
    }
    
    
    void modoAutomatico() {
        //Primero preguntamos si quiere enviar un archivo[0] o ejecutar un archivo ya enviado[1]
        
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        
        bool enviar=false, ejecutar=false;
        
        std::string respuesta;
        std::cout<<"Desea enviar un archivo [1] o ejecutar un archivo ya enviado [2]? ";
        std::getline(std::cin,respuesta);
        
        
        if (respuesta[0]!='1' && respuesta[0]!='2'){
            std::cout<<"Entrada invalida\n";
            return;
        }
        if (respuesta[0]=='1') enviar = true;
        else ejecutar = true;
        
        
        std::string nombreArchivo;
        std::cout << "Ingrese el nombre del archivo para " << (enviar?"enviar: ":"ejecutar: ");
        std::getline(std::cin, nombreArchivo);
        
        if (enviar){
            int resultadoEnvio = cliente->enviarArchivo(nombreArchivo);
            
            if (resultadoEnvio == 0) {
                std::cout << "Archivo enviado exitosamente." << std::endl;
            }
            else if (resultadoEnvio == 1){
                std::cout << "El archivo ya existe, desea sobreescribirlo en el destino? [y/n] ";
                std::getline(std::cin, respuesta);
                
                if (respuesta[0] == 'y' || respuesta[0] == 'Y'){
                    resultadoEnvio = cliente->enviarArchivo(nombreArchivo, true);
                    if (resultadoEnvio != 0){
                        std::cout << "Error al enviar el archivo." << std::endl;
                        return;
                    }
                    else{
                        std::cout << "Archivo enviado exitosamente." << std::endl;
                    }
                }
                else if (respuesta[0]=='n' || respuesta[0]=='N') return;
                else{
                    std::cout << "Opcion invalida, no se envio el archivo\n";
                    return;
                }
            }
            else {
                std::cout << "Error al enviar el archivo." << std::endl;
                return;
            }
            std::cout<<"Desea ejecutar el archivo enviado? [y/n] ";
            std::getline(std::cin, respuesta);
            if (respuesta[0] == 'y' || respuesta[0] == 'Y') ejecutar = true;
        }
        
        if (ejecutar == true){
            int resultadoEjecucion = cliente->ejecutarArchivo(nombreArchivo);
            if (resultadoEjecucion == 0) {
                std::cout << "Archivo ejecutado exitosamente." << std::endl;
            } else {
                std::cout << "Error al ejecutar el archivo." << std::endl;
            }
        }
    }

    void modoManual() {
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        //El programa recibe 3 o 4 números en una misma línea y los convierte a float
        while (true){
            std::string entrada;
            std::cout << "Ingrese las coordenadas , y opcionalmente la velocidad (x y z <v>):"<<std::endl;
            std::getline(std::cin, entrada);
            //Hay que partir la entrada en palabras separadas por espacios, y luego convertir los strings resultantes en números flotantes
            std::stringstream ss(entrada);
            
            std::vector<std::string> str_numeros;
            std::string snum;
            float num;
            float numeros[4];
            int count = 0;
            
            
            while (ss>>snum){
                if (count==4){
                    std::cout<<"Error: Exceso de argumentos, el numero maximo es 4.";
                    return;
                }
                
                try{
                    num = stof(snum);
                }
                catch (std::invalid_argument){
                    std::cout<<"Error: Una de las entradas no es un numero."<<std::endl;
                    return;
                }
                catch(std::out_of_range){
                    std::cout<<"Error: Numero fuera de rango"<<std::endl;
                    return;
                }
                numeros[count]=num;
                ++count;
            }
            if (count < 3){
                std::cout<< "Error: Numero de argumentos insuficiente, el minimo es 3"<<std::endl;
                return;
            }
            
            std::string resultado;
            if (count == 3){
                #ifdef DEBUG
                std::cout<<"x:"<<numeros[0]<<", y:"<<numeros[1]<<", z:"<<numeros[2]<<std::endl;
                #endif
                resultado = cliente->moverXYZ(numeros[0], numeros[1], numeros[2]);
            }
            else{
                #ifdef DEBUG
                std::cout<<"x:"<<numeros[0]<<", y:"<<numeros[1]<<", z:"<<numeros[2]<<", v:"<<numeros[4]<<std::endl;
                #endif
                resultado = cliente->moverXYZ(numeros[0], numeros[1], numeros[2], numeros[3]);
            } 
            std::cout<<resultado<<std::endl;
            
            std::string continuar;
            std::cout << "Desea realizar otro movimiento? (Y/N): ";
            
            //std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            
            std::cin >> continuar;
            
            // Limpiar el buffer de nuevo para prevenir problemas con getline en siguientes iteraciones
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            
            if (continuar[0] == 'Y' || continuar[0] == 'y'){
                continue;
            }
            if (continuar[0] == 'N' || continuar[0] == 'n') {
                break;
            }
            else{
                std::cout << "Entrada invalida\n";
                return;
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
        std::cout << "Seleccionar modo: selecModo (a: automatico, m: manual)" << std::endl;
        std::cout << "Mostrar reporte: reporteGeneral" << std::endl;
        std::cout << "Log de Trabajo del Servidor: pedirLog" << std::endl;
        std::cout << "Home: home" << std::endl;
        std::cout << "Salir del programa: exit" << std::endl;
        std::cout << "--------------------------------------------------------------" << std::endl;
    }

    void mostrarMan() {
        std::cout << "selecModo" << std::endl;
        std::cout << "    Cambiar el modo de operacion del robot entre automatico o manual usando las letras a o m. Por defecto se inicializa en modo manual." << std::endl;
        std::cout << "encenderMotor" << std::endl;
        std::cout << "    Activar los motores del robot" << std::endl;
        std::cout << "apagarMotor" << std::endl;
        std::cout << "    Desactivar los motores del robot" << std::endl;
        std::cout << "reporteGeneral" << std::endl;
        std::cout << "    Mostrar reporte con:" << std::endl;
        std::cout << "    Conteniendo el estado de la conexion" << std::endl;
        std::cout << "    Posicion y estado de actividad actual" << std::endl;
        std::cout << "    Momento en que se inicio la actividad" << std::endl;
        std::cout << "    Listado con el detalle de las ordenes." << std::endl;
        std::cout << "reporteLog" << std::endl;
        std::cout << "    Mostrar el log de trabajo del servidor." << std::endl;
        std::cout << "--------------------------------------------------------------" << std::endl;
    }
};

class Menu{
public:
    void iniciar();
    Menu():sec(false), estadoIP(1){}
private:
    Robot robot;
    std::string comando, usuario,contrasena;
    bool sec;
    char modo;
    int estadoIP;
};

void Menu::iniciar(){
    while (true) {
        comando.clear();
        std::cout << ">>> ";
        std::cin >> comando;
        
        if (comando == "man") {
            robot.mostrarMan();
        } else if (comando == "help") {
            robot.mostrarAyuda();
        }
        
        
        else if (comando=="conectarIP") {
            estadoIP=robot.conectarIP();
        } else if (comando=="desconectarIP"){
            estadoIP=robot.desconectarIP();
        } 
        
        
        else if (comando == "conectarSerie"){
            if (sec){
                robot.conectarSerie();
            } else {
                std::cout<<"Debe registrarse para realizar esta accion"<<std::endl;
            }
        } else if (comando == "desconectarSerie"){
            if (sec){
                robot.desconectarSerie();
            } else {
                std::cout<<"Debe registrarse para realizar esta accion"<<std::endl;
            } 
        }
        
        
        else if(comando=="logIn"){
            if (estadoIP==0){
                while (!sec) {
                    std::cout << "Ingrese usuario: ";
                    std::cin >> usuario;
                    std::cout << "Ingrese contrasenia: ";
                    std::cin >> contrasena;
                    sec = robot.usuarioContrasenia(usuario, contrasena);
                    if (!sec) {
                        std::cout << "Usuario o contrasenia incorrectos. Intente nuevamente." << std::endl;
                    } else {
                        std::cout<<"Se ingreso de manera correcta"<<std::endl;
                    }
                } 
            } else{
                std::cout<<"Para registrarse necesita estar conectado al servido IP"<<std::endl;
            }
        }
        
        
        else if (comando == "selecModo") {
            if (sec){
                std::cout<<"automatico [a] o manual [m]?\n";
                std::cin >> modo;
                robot.selecModo(modo);
            } else {
                std::cout<<"Debe registrarse para realizar esta accion"<<std::endl;
            }  
        } 
        
        
        else if (comando == "encenderMotor") {
            if (sec){
                robot.encenderMotor();
            } else {
                std::cout<<"Debe registrarse para realizar esta accion"<<std::endl;
            }
            
        } else if (comando == "apagarMotor") {
            if (sec){
                robot.apagarMotor();
            } else {
                std::cout<<"Debe registrarse para realizar esta accion"<<std::endl;
            }
            
        } 
        
        
        else if (comando == "home") {
            if (sec){
                robot.home();
            } else {
                std::cout<<"Debe registrarse para realizar esta accion"<<std::endl;
            }
        } else if (comando == "exit") {
            std::cout << "Saliendo del programa." << std::endl;
            break;
        } 
        
        else if (comando == "pedirLog") {
            if (sec){
                std::cout << "Log del servidor:\n" << robot.pedirLog() << std::endl;
            } else {
                std::cout<<"Debe registrarse para realizar esta accion"<<std::endl;
            }
        } else if (comando == "reporteGeneral") {
            if (sec){
                std::cout << "Reporte general:\n" << robot.reporteGeneral() << std::endl;
            } else {
                std::cout<<"Debe registrarse para realizar esta accion"<<std::endl;
            }
        }
        
        else {
            std::cout << "Comando no reconocido. Escriba 'help' para ver los comandos disponibles." << std::endl;
        }
    }
}

int main() {
    Menu menu;
    menu.iniciar();
    
    return 0;
}
