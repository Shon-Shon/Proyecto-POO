
//En esta agregué serialización y deserializador de objetos Lectura y Usuario que había implementado en mockcliente.cpp cuando
// aún no sabía como usar sockets en windows

#include <utility> // Para std::move
#include <cstring>
#include <string>
#include <iostream>
#include <unistd.h>
#include <memory>
#include <prueba_sistema.h>
#if defined(PLATFORM_LINUX)
#include <netinet/in.h>  //Linux
#include <sys/socket.h> //Linux
#include <arpa/inet.h>  //Linux
#elif defined(PLATFORM_WINDOWS)
#include <Ws2tcpip.h>  //Windows
#include <winsock2.h>  //Windows
#endif

//JsonRpc
#include <jsonrpc-lean/client.h>


/* aprendizajes:
Las funciones no pueden distinguir entre literales y constantes, si a una función que acepta punteros modificables le das
un puntero a un array constante (como un literal), algunos compiladores te darán un error, PERO OTROS NO, así que
si tratás con arrays y se supone que tu función debe poder modificarlos conviene crear una sobrecarga para punteros
a constantes que lance un error con certeza sin importar el compilador.
Los valores que devuelve una función son r-values, y de tenerlo la clase a la que le asignas ese valor usará la versión move
de sus funciones para tratar con ese valor, excepto cuando la función devuelve una referencia, en ese caso es deber
del programador escribir si esa referencia será de tipo r-value reference o l-value reference, lo más común es
devolverr l-value reference.
*/


std::string CharToStr(char *entrada){std::string str(entrada); return str;}

class Usuario{
    std::string usuario;
    std::string contrasenia;
    
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
        return objeto;
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
        //std::cout<<"Es Struct (valor): " << valor.IsStruct() <<std::endl;
        auto mapa = valor.AsStruct();
        if (!(mapa["clase"].AsString()=="Lectura")){throw std::domain_error( "se recibio un objeto de clase erronea");}
        //std::cout<<"Es Struct (atributos): " << mapa["atributos"].IsStruct() <<std::endl;
        auto atributos = mapa["atributos"].AsStruct();
        Lectura instancia(0,"");
        // Aquí asumiendo que `mapa` tiene los datos necesarios
        //std::cout<< "numero en atributos: "<< ((atributos.find("numero") == atributos.end())? false:true) <<std::endl;
        //std::cout<< "tiempo en atributos: "<< ((atributos.find("tiempo") == atributos.end())? false:true) <<std::endl;
        //std::cout<< "numero es int32: "<<atributos["numero"].IsInteger32()<<std::endl;
        //std::cout<< "tiempo es string: "<<atributos["tiempo"].IsString()<<std::endl;
        instancia.numero = atributos["numero"].AsInteger32();
        //std::cout<<"convercion correcta de numero"<<std::endl;
        instancia.tiempo = atributos["tiempo"].AsString();
        //std::cout<<"convercion correcta de tiempo"<<std::endl

        //Prueba con el formato de fecha
        //std::istringstream flujo(instancia.tiempo);
        //std::tm prueba = {};
        //flujo>>std::get_time(&prueba, "%a %b %d %H:%M:%S %Y");
        //std::cout<<prueba.tm_hour<<":"<<prueba.tm_min<<":"<<prueba.tm_sec<<std::endl;
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

int main()
{
    #ifdef PLATFORM_WINDOWS
    WSADATA wsaData; //windows
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) { //windows
        std::cerr << "Fallo al iniciar WSAStartup." << std::endl;
        return 1;
    }
    #endif


    // creating socket
    int clientSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (clientSocket == INVALID_SOCKET) {
        std::cerr << "Fallo en la creacion del socket." << std::endl;
        WSACleanup(); //windows
        return 1;
    }
    
    //const char addRequest[] = "{\"jsonrpc\":\"2.0\",\"method\":\"add\",\"id\":0,\"params\":[3,2]}";
    
    // dirección a la que vamos a conectarnos
    sockaddr_in serverAddress;
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(5000);
    serverAddress.sin_addr.s_addr = inet_addr("127.0.0.1");

    // intentamos conectarnos
    if (connect(clientSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) == SOCKET_ERROR) {
        std::cerr << "Fallo al conectar." << std::endl;
        closesocket(clientSocket);
        #ifdef PLATFORM_WINDOWS
        WSACleanup(); //windows
        #endif
        return 1;
    }
    
    //jsonrpc
    std::unique_ptr<jsonrpc::FormatHandler> formatHandler(new jsonrpc::JsonFormatHandler());
    jsonrpc::Client client(*formatHandler);
    
    std::cout<<"--------------------------\n";
    // enviar pedido de procesamiento
    jsonrpc::Request::Parameters params;
    {
        jsonrpc::Value::Struct a;
        a["x"]=3;
        a["y"]=2;
        params.push_back(std::move(a));
    }
    std::shared_ptr<jsonrpc::FormattedData> jsonRequest = client.BuildRequestData("add", params);
    //params.clear();
    std::cout<< "Se envio: " <<jsonRequest->GetData()<<std::endl;
    send(clientSocket, jsonRequest->GetData(), strlen(jsonRequest->GetData()), 0);
    //recibir la respuesta
    char buffer[1024];
    int bytesRecibidos = recv(clientSocket, buffer, sizeof(buffer) - 1, 0);
    if (bytesRecibidos > 0) {
        buffer[bytesRecibidos] = '\0'; // Asegúrate de terminar la cadena
    } else {
        std::cerr << "Fallo en la recepcion del mensaje." << std::endl;
    }
    std::cout<< "Se recibio: " <<CharToStr(buffer) <<std::endl;
    //parsear y mostrar
    jsonrpc::Response parsedResponse = client.ParseResponse(buffer);
    std::cout << "Parsed response: " << parsedResponse.GetResult().AsInteger32() << std::endl;
    
    
    std::cout<<"--------------------------\n";
    jsonRequest=client.BuildRequestData("subtract", 2, 3); //.reset(client.BuildRequestData("substract", 2, 3));
    std::cout<< "Se envio: " <<jsonRequest->GetData()<<std::endl;
    send(clientSocket, jsonRequest->GetData(), strlen(jsonRequest->GetData()), 0);
    bytesRecibidos = recv(clientSocket, buffer, sizeof(buffer) - 1, 0);
    if (bytesRecibidos > 0) {
        buffer[bytesRecibidos] = '\0'; // Asegúrate de terminar la cadena
    } else {
        std::cerr << "Fallo en la recepcion del mensaje." << std::endl;
    }
    std::cout<< "Se recibio: " <<CharToStr(buffer) <<std::endl;
    parsedResponse = client.ParseResponse(buffer);
    std::cout << "Parsed response: " << parsedResponse.GetResult().AsInteger32() << std::endl;
    
    
    std::cout<<"--------------------------\n";
    jsonRequest=client.BuildRequestData("medir"); //.reset(client.BuildRequestData("substract", 2, 3));
    std::cout<< "Se envio: " <<jsonRequest->GetData()<<std::endl;
    send(clientSocket, jsonRequest->GetData(), strlen(jsonRequest->GetData()), 0);
    bytesRecibidos = recv(clientSocket, buffer, sizeof(buffer) - 1, 0);
    if (bytesRecibidos > 0) {
        buffer[bytesRecibidos] = '\0'; // Asegúrate de terminar la cadena
    } else {
        std::cerr << "Fallo en la recepcion del mensaje." << std::endl;
    }
    std::cout<< "Se recibio: " <<CharToStr(buffer) <<std::endl;
    parsedResponse = client.ParseResponse(buffer);
    Lectura lect(0,"");
    deserializarObjeto(lect,parsedResponse.GetResult());
    std::cout << "Parsed response:\n" "numero: "<< lect.numero <<"\ntiempo: " << lect.tiempo << std::endl;
    
    std::cout<<"--------------------------\n";
    params.clear();
    {
        jsonrpc::Value::Struct a;
        a["x"]=3;
        a["y"]=2;
        params.push_back(std::move(a));
        jsonrpc::Value::Struct b;
        params.push_back(std::move(b));
    }
    jsonRequest=client.BuildRequestData("printear", params); //.reset(client.BuildRequestData("substract", 2, 3));
    std::cout<< "Se envio: " <<jsonRequest->GetData()<<std::endl;
    send(clientSocket, jsonRequest->GetData(), strlen(jsonRequest->GetData()), 0);
    bytesRecibidos = recv(clientSocket, buffer, sizeof(buffer) - 1, 0);
    if (bytesRecibidos > 0) {
        buffer[bytesRecibidos] = '\0'; // Asegúrate de terminar la cadena
    } else {
        std::cerr << "Fallo en la recepcion del mensaje." << std::endl;
    }
    std::cout<< "Se recibio: " <<CharToStr(buffer) <<std::endl;
    parsedResponse = client.ParseResponse(buffer);
    std::cout << "Parsed response: " << parsedResponse.GetResult().AsString() << std::endl;
    
    // cerrar socket
    #ifdef PLATFORM_WINDOWS
    closesocket(clientSocket);//windows
    WSACleanup(); //windows
    #else
    close(clientSocket); //linux
    #endif


    return 0;
}
