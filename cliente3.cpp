#include <utility> // Para std::move
#include <cstring>
#include <cerrno>
#include <string>
#include <iostream>
#include <unistd.h>
#include <memory>
#include <prueba_sistema.h>
#include <fstream>
#include <sstream>
//particulares de cada sistema 
#if defined(PLATFORM_LINUX)
#include <netinet/in.h>  //Linux
#include <sys/socket.h> //Linux
#include <arpa/inet.h>  //Linux
#elif defined(PLATFORM_WINDOWS)
#include <Ws2tcpip.h>  //Windows
#include <winsock2.h>  //Windows
#include <windows.h>
#endif

//JsonRpc
#include <jsonrpc-lean/client.h>

/* aprendizajes:
Por alguna razón este modulo solo anda si construis Cliente en un puntero como
Clinete* cliente = new Cliente();
No si lo intentas construir en una variable normal
*/

const unsigned int tiempo_timeout = 100;
//#define DEBUG

class Cliente;
class Lectura;
class Usuario;

class Cliente{
public:
    int conectarIP();
    void desconectarIP();
    
    void guardarUsuario(std::string usuario, std::string contrasenia);
    bool verificarUsuario();
    int conectarSerie();
    void desconectarSerie();
    
    void selecModo(char modo);
    
    void encenderMotor();
    void apagarMotor();
    std::string moverXYZ(double x, double y, double z);
    std::string moverXYZ(double x, double y, double z, double v);
    void home();
    
    //archivos modo automatico
    int enviarArchivo(const std::string& nombreArchivo, bool sobreescribir = false);
    int ejecutarArchivo(const std::string& nombreArchivo);
    
    //reportes
    std::string pedirLog();
    std::string pedirRegistro();
    
    Cliente();
    
private:
    //socket
    #ifdef PLATFORM_WINDOWS
    WSADATA wsaData;
    #endif
    int clientSocket;
    bool conectadoIP = 0;
    char buffer[1024];
    char buffer_salida[1024];
    const int bufferSize = 1024;
    
    //jsonrpc
    std::unique_ptr<jsonrpc::FormatHandler> formatHandler;
    jsonrpc::Client client;
    jsonrpc::Request::Parameters params;
    std::shared_ptr<jsonrpc::FormattedData> jsonRequest;
    
    //usuario
    std::unique_ptr<Usuario> usuario_guardado;
};



std::string CharToStr(char *entrada){std::string str(entrada); return str;}

class Usuario{
    std::string usuario;
    std::string contrasenia;
    
public:
    Usuario(std::string _usuario, std::string _constrasenia): usuario(_usuario), contrasenia(_constrasenia){}
    
    Usuario(Usuario&& otro) noexcept{
        if (this != &otro){
            usuario = std::move(otro.usuario);
            contrasenia = std::move(otro.contrasenia);
        }
    }
    
    Usuario& operator=(Usuario&& otro) noexcept {
        if (this != &otro) {
            usuario = std::move(otro.usuario);
            contrasenia = std::move(contrasenia);
        }
        return *this;
    }
    
    jsonrpc::Value serializar(){
        jsonrpc::Value::Struct objeto;
        objeto["clase"] = "Usuario";
        jsonrpc::Value::Struct atributos;
        atributos["usuario"]=this->usuario;
        atributos["contrasenia"]=this->contrasenia;
        objeto["atributos"]=atributos;
        return objeto; //Aquí el compilador aplicará RVO para construir 'objeto' en la dirección de destino de esta función
    }
};

class Lectura{
public:
    int numero;
    std::string tiempo;
    Lectura(int _numero, std::string _tiempo):numero(_numero),tiempo(_tiempo){}
    
    Lectura(Lectura&& otro) noexcept{
        if (this != &otro){
            numero = std::move(otro.numero);
            tiempo = std::move(otro.tiempo);
        }
    }
    
    Lectura& operator=(Lectura&& otro) noexcept {
        if (this != &otro) {
            numero = std::move(otro.numero);
            tiempo = std::move(tiempo);
        }
        return *this;
    }
    
    static Lectura deserializar(const jsonrpc::Value& valor){
        auto mapa = valor.AsStruct();
        if (!(mapa["clase"].AsString()=="Lectura")){throw std::domain_error( "se recibio un objeto de clase erronea");}
        auto atributos = mapa["atributos"].AsStruct();
        Lectura instancia(0,"");
        instancia.numero = atributos["numero"].AsInteger32();
        instancia.tiempo = atributos["tiempo"].AsString();
        return instancia;
    }
};

template <typename T>
void deserializarObjeto(T& objeto, const jsonrpc::Value& valor) {
    objeto = std::move(T::deserializar(valor));
}

template <typename T>
jsonrpc::Value serializarObjeto(T& objeto) {
    return objeto.serializar();
}

//Funciones de clases

///////////////Cliente/////////////////

Cliente::Cliente():
        formatHandler(std::make_unique<jsonrpc::JsonFormatHandler>()),
        client(*formatHandler) //client no tiene operador de asignación, así que hay que usar el constructor acá
{
    #ifdef DEBUG
    std::cout<<"Cliente()"<<std::endl;
    #endif
}

int Cliente::conectarIP(){
    #ifdef DEBUG
    std::cout<<"conectarIP()"<<std::endl;
    #endif
    #ifdef PLATFORM_WINDOWS
    //WSADATA wsaData; //windows
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) { //windows
        std::cerr << "Fallo al iniciar WSAStartup." << std::endl;
        return 1;
    }
    #endif
    
    // creating socket
    clientSocket = socket(AF_INET, SOCK_STREAM, 0);
    #ifdef PLATFORM_WINDOWS
    if (clientSocket == INVALID_SOCKET) {
    #else
    if (clientSocket == -1) {
    #endif
        std::cerr << "Fallo en la creación del socket: " << std::strerror(errno) << std::endl;
        #ifdef PLATFORM_WINDOWS
        WSACleanup(); //windows
        #endif
        return 1;
    }
    
    // dirección a la que vamos a conectarnos
    sockaddr_in serverAddress;
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(5000);
    serverAddress.sin_addr.s_addr = inet_addr("127.0.0.1");
    
    // intentamos conectarnos WSAGetLastError()
    #ifdef PLATFORM_WINDOWS
    if (connect(clientSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) == SOCKET_ERROR) {
        std::cerr << "Fallo al conectar: " << std::strerror(WSAGetLastError()) << std::endl;
        closesocket(clientSocket);
        WSACleanup(); //windows
        return 1;
    }
    #else
    if (connect(clientSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) < 0) {
        std::cerr << "Fallo al conectar: " << std::strerror(errno) << std::endl;
        close(clientSocket); // Usar close en Unix
        return 1;
    }
    #endif
    
    return 0;
}

void Cliente::desconectarIP(){
    #ifdef DEBUG
    std::cout<<"desconectarIP()"<<std::endl;
    #endif
    // cerrar socket
    #ifdef PLATFORM_WINDOWS
    closesocket(clientSocket);//windows
    WSACleanup(); //windows
    #else
    close(clientSocket); //linux
    #endif
}

void Cliente::guardarUsuario(std::string usuario, std::string contrasenia){
    #ifdef DEBUG
    std::cout<<"guardarUsuario("+usuario+", "+contrasenia+")"<<std::endl;
    #endif
    usuario_guardado = std::make_unique<Usuario>(usuario, contrasenia);
}

bool Cliente::verificarUsuario(){
    #ifdef DEBUG
    std::cout<<"verificarUsuario()"<<std::endl;
    #endif
    if (usuario_guardado==nullptr) throw std::runtime_error("No se ha guardado ningun usuario aun");
    jsonRequest = client.BuildRequestData("validar_usuario", serializarObjeto(*usuario_guardado));
    
    //std::cout<< "Se envio: " <<jsonRequest->GetData()<<std::endl;
    send(clientSocket, jsonRequest->GetData(), strlen(jsonRequest->GetData()), 0);
    
    int bytesRecibidos = recv(clientSocket, buffer, sizeof(buffer) - 1, 0);
    if (bytesRecibidos > 0) {
        buffer[bytesRecibidos] = '\0'; // Asegúrate de terminar la cadena
    } else {
        std::cerr << "Fallo en la recepcion del mensaje:" << strerror(errno) << std::endl;
    }
    //std::cout<< "Se recibio: " <<CharToStr(buffer) <<std::endl;
    jsonrpc::Response parsedResponse = client.ParseResponse(buffer);
    return parsedResponse.GetResult().AsBoolean();
}

//////Funciones con el robot////////

int Cliente::conectarSerie(){
    #ifdef DEBUG
    std::cout<<"conectarSerie()"<<std::endl;
    #endif
    jsonRequest = client.BuildRequestData("conectarSerie");
    send(clientSocket, jsonRequest->GetData(), strlen(jsonRequest->GetData()), 0);
    
    int bytesRecibidos = recv(clientSocket, buffer, sizeof(buffer) - 1, 0);
    if (bytesRecibidos > 0) {
        buffer[bytesRecibidos] = '\0'; // Asegúrate de terminar la cadena
    } else {
        std::cerr << "Fallo en la recepcion del mensaje:" << strerror(errno) << std::endl;
        return -1;
    }
    jsonrpc::Response parsedResponse = client.ParseResponse(buffer);
    return parsedResponse.GetResult().AsInteger32();
}

void Cliente::desconectarSerie(){
    #ifdef DEBUG
    std::cout<<"desconectarSerie()"<<std::endl;
    #endif
    jsonRequest = client.BuildNotificationData("desconectarSerie");
    send(clientSocket, jsonRequest->GetData(), strlen(jsonRequest->GetData()), 0);
    //eliminar
    #ifdef PLATFORM_WINDOWS
    Sleep(100);
    #else
    usleep(static_cast<int>(100000)); //100 ms = 100 000 us 
    #endif
}

void Cliente::selecModo(const char modo){
    #ifdef DEBUG
    std::cout<<"selecModo("+std::string(1,modo)+")"<<std::endl;
    #endif
    //código para seleccionar modo
    jsonRequest = client.BuildNotificationData("selecModo", std::string(1, modo));
    send(clientSocket, jsonRequest->GetData(), strlen(jsonRequest->GetData()), 0);
    //eliminar
    #ifdef PLATFORM_WINDOWS
    Sleep(100);
    #else
    usleep(static_cast<int>(100000)); //100 ms = 100 000 us 
    #endif
}

void Cliente::encenderMotor(){
    #ifdef DEBUG
    std::cout<<"encenderMotor()"<<std::endl;
    #endif
    jsonRequest = client.BuildNotificationData("encenderMotor");
    send(clientSocket, jsonRequest->GetData(), strlen(jsonRequest->GetData()), 0);    
    //eliminar
    #ifdef PLATFORM_WINDOWS
    Sleep(100);
    #else
    usleep(static_cast<int>(100000)); //100 ms = 100 000 us 
    #endif
}

void Cliente::apagarMotor(){
    #ifdef DEBUG
    std::cout<<"apagarMotor()"<<std::endl;
    #endif
    jsonRequest = client.BuildNotificationData("apagarMotor");
    send(clientSocket, jsonRequest->GetData(), strlen(jsonRequest->GetData()), 0);
    //eliminar
    #ifdef PLATFORM_WINDOWS
    Sleep(100);
    #else
    usleep(static_cast<int>(100000)); //100 ms = 100 000 us 
    #endif
}

std::string Cliente::moverXYZ(double x, double y, double z){
    #ifdef DEBUG
    std::cout<<"moverXYZ("<<x<<","<<y<<","<<z<<")"<<std::endl;
    #endif
    jsonRequest = client.BuildRequestData("moverXYZ", x, y, z);
    send(clientSocket, jsonRequest->GetData(), strlen(jsonRequest->GetData()), 0);
    
    int bytesRecibidos = recv(clientSocket, buffer, sizeof(buffer) - 1, 0);
    if (bytesRecibidos > 0) {
        buffer[bytesRecibidos] = '\0'; // Asegúrate de terminar la cadena
    } else {
        std::cerr << "Fallo en la recepcion del mensaje:" << strerror(errno) << std::endl;
    }
    jsonrpc::Response parsedResponse = client.ParseResponse(buffer);
    return parsedResponse.GetResult().AsString();
}

std::string Cliente::moverXYZ(double x, double y, double z, double v){
    #ifdef DEBUG
    std::cout<<"moverXYZ("<<x<<","<<y<<","<<z<<","<<v<<")"<<std::endl;
    #endif
    jsonRequest = client.BuildRequestData("moverXYZ", x, y, z, v);
    send(clientSocket, jsonRequest->GetData(), strlen(jsonRequest->GetData()), 0);
    
    int bytesRecibidos = recv(clientSocket, buffer, sizeof(buffer) - 1, 0);
    if (bytesRecibidos > 0) {
        buffer[bytesRecibidos] = '\0'; // Asegúrate de terminar la cadena
    } else {
        std::cerr << "Fallo en la recepcion del mensaje:" << strerror(errno) << std::endl;
    }
    jsonrpc::Response parsedResponse = client.ParseResponse(buffer);
    return parsedResponse.GetResult().AsString();
}

void Cliente::home(){
    #ifdef DEBUG
    std::cout<<"home()"<<std::endl;
    #endif
    jsonRequest = client.BuildNotificationData("home");
    send(clientSocket, jsonRequest->GetData(), strlen(jsonRequest->GetData()), 0);
    //eliminar
    #ifdef PLATFORM_WINDOWS
    Sleep(100);
    #else
    usleep(static_cast<int>(100000)); //100 ms = 100 000 us 
    #endif
}

int Cliente::enviarArchivo(const std::string& nombreArchivo, bool sobreescribir){
    #ifdef DEBUG
    std::cout<<"enviarArchivo("+nombreArchivo+","<<(sobreescribir?"true":"false")<<")"<<std::endl;
    #endif
    std::ifstream inFile(nombreArchivo, std::ios::binary);
    if (!inFile) {
        std::cerr << "No se pudo abrir el archivo para leer." << std::endl;
        return -1;
    }

    jsonRequest = client.BuildRequestData("enviarArchivo", nombreArchivo, sobreescribir);    
    send(clientSocket, jsonRequest->GetData(), strlen(jsonRequest->GetData()), 0);
    
    int bytesRecibidos = recv(clientSocket, buffer, sizeof(buffer) - 1, 0);
    if (bytesRecibidos > 0) {
        buffer[bytesRecibidos] = '\0'; // Asegúrate de terminar la cadena
    } else {
        std::cerr << "Fallo en la recepcion del mensaje:" << strerror(errno) << std::endl;
        return -1;
    }
    jsonrpc::Response parsedResponse = client.ParseResponse(buffer);
    if (parsedResponse.GetResult().AsInteger32() > 0){
        return parsedResponse.GetResult().AsInteger32();
    }
    
    while (inFile.read(buffer, bufferSize) || inFile.gcount() > 0) {
        size_t bytesLeidos = inFile.gcount();
        //Construimos un pedido a la función seguir_enviando
        jsonRequest = client.BuildNotificationData("seguir_enviando", std::string(buffer, bytesLeidos));
        send(clientSocket, jsonRequest->GetData(), strlen(jsonRequest->GetData()), 0);
        //eliminar
        #ifdef PLATFORM_WINDOWS
        Sleep(100);
        #else
        usleep(static_cast<int>(100000)); //100 ms = 100 000 us 
        #endif
    }
    
    jsonRequest = client.BuildNotificationData("terminar_de_enviar");
    send(clientSocket, jsonRequest->GetData(), strlen(jsonRequest->GetData()), 0);
    //eliminar
    #ifdef PLATFORM_WINDOWS
    Sleep(100);
    #else
    usleep(static_cast<int>(100000)); //100 ms = 100 000 us 
    #endif
    
    //inFile.close();
    //std::cout << "Archivo enviado." << std::endl;
    return 0;
}


int Cliente::ejecutarArchivo(const std::string& nombreArchivo){
    #ifdef DEBUG
    std::cout<<"ejecutarArchivo("+nombreArchivo+")"<<std::endl;
    #endif
    jsonRequest = client.BuildRequestData("ejecutarArchivo", nombreArchivo);
    send(clientSocket, jsonRequest->GetData(), strlen(jsonRequest->GetData()), 0);
    
    int bytesRecibidos = recv(clientSocket, buffer, sizeof(buffer) - 1, 0);
    if (bytesRecibidos > 0) {
        buffer[bytesRecibidos] = '\0'; // Asegúrate de terminar la cadena
    } else {
        std::cerr << "Fallo en la recepcion del mensaje:" << strerror(errno) << std::endl;
        return -1;
    }
    jsonrpc::Response parsedResponse = client.ParseResponse(buffer);
    return parsedResponse.GetResult().AsInteger32();
}


std::string Cliente::pedirLog(){
    #ifdef DEBUG
    std::cout<<"pedirLog()"<<std::endl;
    #endif
    jsonRequest = client.BuildRequestData("pedirLog");
    send(clientSocket, jsonRequest->GetData(), strlen(jsonRequest->GetData()), 0);
    
    int bytesRecibidos;
    std::ostringstream oss;
    
    fd_set readfds;
    struct timeval timeout;
    while (true) {
        FD_ZERO(&readfds);
        FD_SET(clientSocket, &readfds);
        // Configura el timeout
        timeout.tv_sec = 0;
        timeout.tv_usec = 100000; //100 ms
        
        int activity = select(clientSocket + 1, &readfds, nullptr, nullptr, &timeout);
        if (activity < 0) {
            std::cerr << "Error en select: " << strerror(errno) << std::endl;
            break;
        } else if (activity == 0) {
            // Timeout alcanzado, consideramos que recibimos todo
            break;
        } else {
            // Hay datos para recibir
            bytesRecibidos = recv(clientSocket, buffer, sizeof(buffer), 0);
            if (bytesRecibidos > 0) {
                oss.write(buffer, bytesRecibidos);
            } else if (bytesRecibidos < 0) {
                std::cerr << "Error al recibir datos: " << strerror(errno) << std::endl;
                break;
            } else {
                // La conexión se cerró
                std::cout << "La conexion se cerro." << std::endl;
                break;
            }
        }
    }
    
    jsonrpc::Response parsedResponse = client.ParseResponse(oss.str());
    return parsedResponse.GetResult().AsString();
}


std::string Cliente::pedirRegistro(){
    #ifdef DEBUG
    std::cout<<"pedirRegistro()"<<std::endl;
    #endif
    jsonRequest = client.BuildRequestData("pedirRegistro");
    send(clientSocket, jsonRequest->GetData(), strlen(jsonRequest->GetData()), 0);
    
    int bytesRecibidos;
    std::ostringstream oss;
    fd_set readfds;
    struct timeval timeout;
    while (true) {
        FD_ZERO(&readfds);
        FD_SET(clientSocket, &readfds);
        // Configura el timeout
        timeout.tv_sec = 0;
        timeout.tv_usec = 100000; //100 ms
        
        int activity = select(clientSocket + 1, &readfds, nullptr, nullptr, &timeout);
        if (activity < 0) {
            std::cerr << "Error en select: " << strerror(errno) << std::endl;
            break;
        } else if (activity == 0) {
            // Timeout alcanzado, consideramos que recibimos todo
            break;
        } else {
            // Hay datos para recibir
            bytesRecibidos = recv(clientSocket, buffer, sizeof(buffer), 0);
            if (bytesRecibidos > 0) {
                oss.write(buffer, bytesRecibidos);
            } else if (bytesRecibidos < 0) {
                std::cerr << "Error al recibir datos: " << strerror(errno) << std::endl;
                break;
            } else {
                // La conexión se cerró
                std::cout << "La conexion se cerro." << std::endl;
                break;
            }
        }
    }
    
    jsonrpc::Response parsedResponse = client.ParseResponse(oss.str());
    return parsedResponse.GetResult().AsString();
}
