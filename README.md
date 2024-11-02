# Proyecto-POO
Proyecto para el integrador de programación orientada a objetos

## Cosas que vamos a hacer:
Interfaz de cliente (c++)  
Interfaz de administrador (python)  
Comunicación cliente servidor <=> Protocolo RPC  
Robot  


Para compilar el archivo cliente3.cpp tienen que tener una carpeta llamada includes en la que tienen que estar las carpetas:  
jsonrpc-lean (se obtiene de la página: https://github.com/uskr/jsonrpc-lean/tree/master/include/jsonrpc-lean)  
rapidjson (se obtiene de la página: https://github.com/Tencent/rapidjson/tree/master/include/rapidjson)  
Si están en windows el comando para compilarlo es:  
`g++ -I ./includes cliente3.cpp -lws2_32 -o cliente3Cpp`  
Si están en linux es:  
`g++ -I ./includes cliente3.cpp -o cliente3Cpp`
