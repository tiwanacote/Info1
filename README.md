Instructivo para correr el TP final de mediciones telemétricas - BARONE/BERTOTTO


1) Instalar librerias


PUBNUB:    pip install 'pubnub>=3,<4'


ZEROmq:    sudo apt-get install libzmq3-dev


2) Abrir 5 consolas en “Terminator”  y pararse en todas en el PATH donde se encuentran los archivos


Consola 1: 	Emula la lectura de los sensores de las Raspberrys. Los datos son enviados vía UDP al servidor escrito en C  


Editar “id_config.txt” y colocar un 1 
gcc -o cliente1 cliente.c sensorlib.h -lmr   
./cliente1




Consola 3:  Idem


Editar “id_config.txt” y colocar un 2  
gcc -o cliente2 cliente.c sensorlib.h -lmr   
./cliente2




Consola 4:   Idem  


Editar “id_config.txt” y colocar un 3
gcc -o cliente3 cliente.c sensorlib.h -lmr 
./cliente3
 
Consola 4:   Aquí correrá el servidor programado en C. Recibe datos de cada raspberry y envía datos a script de Python utilizando ZeroMQ. Primero compilamos, luego lo corremos


 gcc -Wall -g servidor.c -lzmq -o servidor   
./servidor


Consola 5: Recibe datos del server en C y los envía a l servidor de Pubnub


python integracion_1.py  
3) Entrar a freeboard.io y registrarse como:


Usuario:  bertotto.maximiliano@gmail.com
Contraseña: 12345678maxi


Entrar al link que dice “prueba” y luego abajo poner “Full screen”
