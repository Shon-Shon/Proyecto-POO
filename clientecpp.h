#ifndef CLIENTE_CPP_H
#define CLIENTE_CPP_H

#include <string>

class Cliente;

class Cliente_inter{
public:
    void guardarUsuario();
    bool verificarUsuario();
    int conectarIP();
    void desconectarIP();
    
    //robot
    int conectarSerie();
    void desconectarSerie();
    
    void selecModo(char modo);
    
    void encenderMotor();
    void apagarMotor();
    std::string moverXYZ(double x, double y, double z);
    std::string moverXYZ(double x, double y, double z, double v);
    void home();
    
    //archivos modo automatico
    int cargarArchivo(std::string nombre);
    int ejecutarArchivo(std::string nombre);
    
    //reportes
    std::string pedirLog();
    std::string pedirRegistro();
    
private:
    Cliente* cliente;
};

#endif //CLIENTE_CPP_H
