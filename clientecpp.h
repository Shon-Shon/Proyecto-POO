#ifndef CLIENTE_CPP_H
#define CLIENTE_CPP_H

#include <string>

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
};

#endif //CLIENTE_CPP_H
