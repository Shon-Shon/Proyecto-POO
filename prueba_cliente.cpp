#include <iostream>
#include <string>
#include "clientecpp.h"

int main()
{
    std::cout << "Conectando IP..." << std::endl;
    Cliente* cliente = new Cliente();
    cliente->conectarIP();
    
    std::cout<<"--------------------------\n";
    cliente->guardarUsuario("Tomas","lala");
    std::cout<<"Orden: verificarUsuario(Usuario(\"Tomas\", \"lala\"))"<<std::endl;
    bool prueba1 = cliente->verificarUsuario();
    std::cout << "Parsed response: " << (prueba1?"true":"false") << std::endl;
    
    std::cout<<"Orden: conectarSerie()"<<std::endl;
    int prueba2 = cliente->conectarSerie();
    std::cout << "Parsed response: " << prueba2 << std::endl;
    std::cout<<"Orden: desconectarSerie()"<<std::endl;
    cliente->desconectarSerie();
    
    std::cout<<"Orden: selecModo('a')"<<std::endl;
    cliente->selecModo('a');
    std::cout<<"Orden: selecModo('m')"<<std::endl;
    cliente->selecModo('m');
    std::cout<<"Orden: encenderMotor()"<<std::endl;
    cliente->encenderMotor();
    std::cout<<"Orden: apagarMotor()"<<std::endl;
    cliente->apagarMotor();
    
    std::cout<<"Orden: moverXYZ(1,2,3)"<<std::endl;
    std::string prueba3 = cliente->moverXYZ(1.0, 2.0, 3.0); //con x,y,z
    std::cout << "Parsed response: " << prueba3 << std::endl;
    std::cout<<"Orden: moverXYZ(1,2,3,4)"<<std::endl;
    std::string prueba4 = cliente->moverXYZ(1.0, 2.0, 3.0, 4.0); //con x,y,z,v
    std::cout << "Parsed response: " << prueba4 << std::endl;
    
    std::cout<<"Orden: home()"<<std::endl;
    cliente->home();
    
    std::cout<<"Orden: enviarArchivo(\"archivo_prueba.gcode\")"<<std::endl; 
    int prueba5 = cliente->enviarArchivo("archivo_prueba.gcode"); //Este debería andar normal y devolver 0
    std::cout << "Parsed response: " << prueba5 << std::endl;
    std::cout<<"Orden: enviarArchivo(\"archivo_prueba.gcode\")"<<std::endl;
    int prueba6 = cliente->enviarArchivo("archivo_prueba.gcode"); //Este debería devolver 1 porque ya existe el archivo
    std::cout << "Parsed response: " << prueba6 << std::endl;
    std::cout<<"Orden: enviarArchivo(\"archivo_prueba.gcode\", true)"<<std::endl;
    int prueba7 = cliente->enviarArchivo("archivo_prueba.gcode", true); //Este debería devolver 0 y sobreescribir el archivo
    std::cout << "Parsed response: " << prueba7 << std::endl;
    
    std::cout<<"Orden: ejecutarArchivo(\"archivo_prueba.gcode\")"<<std::endl;
    int prueba8 = cliente->ejecutarArchivo("archivo_prueba.gcode"); //Este se debería andar normal y devolver 0
    std::cout << "Parsed response: " << prueba8 << std::endl;
    std::cout<<"Orden: ejecutarArchivo(\"archivo_inexistente.gcode\")"<<std::endl;
    int prueba9 = cliente->ejecutarArchivo("archivo_inexistente.gcode"); //Este debería devolver un num!=0 porque no existe el archivo
    std::cout << "Parsed response: " << prueba9 << std::endl;
    
    std::cout<<"Orden: pedirLog()"<<std::endl;
    std::string prueba10 = cliente->pedirLog();  //devuelve un string muy largo
    std::cout << "Parsed response: " << prueba10 << std::endl;
    std::cout<<"Orden: pedirRegistro()"<<std::endl;
    std::string prueba11 = cliente->pedirRegistro(); //devuelve un string muy largo
    std::cout << "Parsed response: " << prueba11 << std::endl;
    
    cliente->desconectarIP();

    return 0;
}
