//VERSIONES:
/*
v1 - Incorporacion de la funcion registro
v2 - Incorporacion de la funcion imprime
v3 - Deteccion de archivo  de configuracion vacio - ver bug en insertar nombre de insertar sensor
*/
//LIBRERIAS-----------------------------------------------------------------------------------------------
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<ctype.h>
#include<math.h>
#include<time.h>
#include<unistd.h>	
#include<netdb.h>
#include<sys/socket.h>
#include<arpa/inet.h>
//#include<zmq.h>
#include<assert.h>

//DEFINICIONES------------------------------------------------------------------------------------------
#define RPI_ID 2
#define TYPELEN 30				//largo del nombre del tipo de curva
#define NOMLEN 30				//largo del nombre del sensor
#define BUFSIZE 100				//largo del buffer de medicion
#define MAX_SENSOR 5			//numero maximo de sensores conectado en un rpi
#define SAMPLE_RATE 1			//retardo del simulador
#define SERVICE_PORT 21234		//puerto del servidor
#define SERVER_IP "127.0.0.2"	//IP del servidor

//ESTRUCTURAS--------------------------------------------------------------------------------------------------------------
struct curva{
	char tipo[TYPELEN];
	float a;					// Parametro de curva "a"
	float b;					// Parametro de curva "b"
	float c;					// Parametro de curva "c"
	};
	
struct sensor{
	int nro; 					// Numero de sensor conectado a la Raspberry	
	char nombre[NOMLEN];		// Nombre de sensor
	struct curva tipo_curva;		// Curva de interpolacion. Puede ser: lineal, log, exp, polinomica
	struct sensor *ptrSiguiente;
};

typedef struct sensor Sensor; 

//FUNCIONES PRINCIPALES---------------------------------------------------------------------------------------------------

int imprime_sensores(Sensor* ptrInicial){
	//DESCRIPCION: -------------------------------------------------------------------------------------------------------
	/*	Imprime la lista de sensores por pantalla, retorna -1 si la lista esta vacia.-	*/
	
	//CUERPO DE LA FUNCION-------------------------------------------------------------------------------------------------
	printf("\n");	
	if (ptrInicial == NULL)		
		return -1;		//Lista vacia
	else {
		printf("\n-----------------------------------------------------\n");
		while (ptrInicial != NULL) {
		fprintf(stdout,"Numero de Sensor: %d\nNombre de Sensor: %s\nCurva: %s - Parametros A: %.2f B: %.2f C: %.2f\n"
		"-----------------------------------------------------\n", 
		ptrInicial->nro, ptrInicial->nombre, ptrInicial->tipo_curva.tipo, 
		ptrInicial->tipo_curva.a, ptrInicial->tipo_curva.b, ptrInicial->tipo_curva.c);
		ptrInicial = ptrInicial->ptrSiguiente;
		}//fin del while
	}//fin del else
	return 0;
}//fin de la funcion

int ingresar_sensores(Sensor** ptrInicial){
	//DESCRIPCION----------------------------------------------------------------------------------------------------------
	/*	Esta funcion permite ingresar sensores de forma continua por teclado e insertarlos en una lista dinamica simple
		retorna -1 si hay memoria insuficiente para agregar un sensor.-	*/
	
	//FUNCIONES AUXILIARES-------------------------------------------------------------------------------------------------
	void ingresar_sensor(Sensor* ptrS, int num);
	int insertar_sensor(Sensor** ptrInicial, Sensor* ptrNuevo);
	Sensor* buscar_sensor(Sensor* ptrInicial, int num);
	int modificar_sensor(Sensor* ptrInicial, int num);
	
	//VARIABLES------------------------------------------------------------------------------------------------------------
	Sensor* ptrNuevo;				//puntero al nuevo sensor
	Sensor* ptrBusqueda;
	char remp, continuar = 'Y';		//orden para continuar o terminar
	int num;
	
	//CUERPO DE LA FUNCION-------------------------------------------------------------------------------------------------
	while (continuar == 'Y') {
		ptrNuevo = (Sensor *)malloc(sizeof(Sensor));	//pido memoria
		
		//Memoria no asignada
		if (ptrNuevo == NULL)
		return -1;	//Memoria Insuficiente
		
		//Memoria asignada
		else {	
		printf("-----------------------------------------------------\n");
		
		//ingresamos un numero de sensor
		printf("Ingrese nro de sensor: ");
		fflush(stdin);
		scanf("%d", &num);	
		ptrBusqueda = buscar_sensor(*ptrInicial, num);	//buscamos coincidencias en la lista
		
		//numero de sensor libre
		if (ptrBusqueda == NULL){
		ingresar_sensor(ptrNuevo, num);			//ingreso los datos del sensor
		insertar_sensor(ptrInicial, ptrNuevo);	//inserto el sensor en la lista de sensores	
		}//fin del if
		
		//si el sensor ya esta existente
		else {
		printf("Sensor Existente\n");
			//lo modificamos?
			do {
			printf("Desea modificarlo? \"Y/N\": ");	
			scanf("\n%c", &remp);
			remp = toupper(remp);				//convierte a mayusculas
			}//fin del do
			while(remp != 'Y' && remp != 'N' );	//al salir del bucle tenermos una respuesta
			
			//modificamos
			if (remp == 'Y')	
			modificar_sensor(*ptrInicial, num);
			
			//no modificamos	
			else
			free (ptrNuevo);	//liberamos memoria
			}//fin del else
		}//fin del else
		
		//Ingresar otro sensor?
		do {
		printf("\nIngresar otro sensor? \"Y/N\": ");	
		scanf("\n%c", &continuar);
		continuar = toupper(continuar);				//convierte a mayusculas
		}//fin del do
		while(continuar != 'Y' && continuar != 'N' );
	}//fin del while
	return 0;
}//fin de la funcion

int importar_sensores(FILE* ptrArchivo, Sensor** ptrInicial){
	//DESCRIPCION----------------------------------------------------------------------------------------------------------
	/*	Carga los sensores desde el archivo de configuracion. Retorna -1 si el archivo esta vacio
	y -2 si la memoria es insuficiente para cargar un sensor
	ptrArchivo: puntero al archivo registro de sensores
	ptrPrimero: puntero al primer sensor de la lista.-	*/
	
	//FUNCIONES AUXILIARES-------------------------------------------------------------------------------------------------
	int insertar_sensor(Sensor** ptrInicial, Sensor* ptrNuevo);
	int borrar_lista(Sensor** ptrInicial);
	
	//VARIABLES------------------------------------------------------------------------------------------------------------
	Sensor* ptrNuevo;	//puntero al sensor obtenido
	
	//CUERPO DE LA FUNCION-------------------------------------------------------------------------------------------------
	borrar_lista(ptrInicial);		//limpiamos la lista
	fseek( ptrArchivo, 0, SEEK_END );
	if (ftell( ptrArchivo ) == 0 )			//archivo vacio
		return -1;
	rewind(ptrArchivo);
	while (!feof(ptrArchivo)) {		//recorremos el archivo hasta el final
		ptrNuevo = (Sensor *)malloc(sizeof(Sensor));	//pido memoria
		if (ptrNuevo == NULL) {		//Memoria no asignada
		return -2;		//Memoria Insuficiente
		}//fin del if
		else {		//obtenemos los datos de un sensor
			fscanf(ptrArchivo,"Numero de Sensor: %d\nNombre de Sensor: %s\nCurva: %s - Parametros A: %f B: %f C: %f\n"
			"-----------------------------------------------------\n", 
			&ptrNuevo->nro, ptrNuevo->nombre, ptrNuevo->tipo_curva.tipo, 
			&ptrNuevo->tipo_curva.a, &ptrNuevo->tipo_curva.b, &ptrNuevo->tipo_curva.c);
			ptrNuevo->ptrSiguiente = NULL;
			//insertamos el sensor obtenido en la lista de sensores
			insertar_sensor(ptrInicial, ptrNuevo);
		}//fin del else	
	}//fin del while
	fclose(ptrArchivo);
	return 0;	//importacion exitosa
}//fin de la funcion

void exportar_sensores(FILE* ptrArchivo, Sensor* ptrInicial){
	//DESCRIPCION----------------------------------------------------------------------------------------------------------
	/*Esta funcion guarda los sensores de la lista en un archivo de configuracion de sensores.
	ptrArchivo: puntero al archivo destino
	ptrPrimero: puntero a la lista de sensores				*/
	
	//CUERPO DE LA FUNCION-------------------------------------------------------------------------------------------------
		while (ptrInicial != NULL) {
			fprintf(ptrArchivo,"Numero de Sensor: %d\nNombre de Sensor: %s\nCurva: %s - Parametros A: %.2f B: %.2f C: %.2f\n"
			"-----------------------------------------------------\n", 
			ptrInicial->nro, ptrInicial->nombre, ptrInicial->tipo_curva.tipo, 
			ptrInicial->tipo_curva.a, ptrInicial->tipo_curva.b, ptrInicial->tipo_curva.c);
			ptrInicial = ptrInicial->ptrSiguiente;	//avance
			}//fin del while interno
		fclose(ptrArchivo);

}//fin de la funcion exportar sensores

int borrar_lista(Sensor** ptrInicial){
	//DESCRIPCION----------------------------------------------------------------------------------------------------------
	/*Esta funcion borra la lista de sensores.
	ptrinicial: puntero a la lista de sensores				*/
	
	//VARIABLES------------------------------------------------------------------------------------------------------------
	Sensor* ptrTemp;
	
	//CUERPO DE LA FUNCION-------------------------------------------------------------------------------------------------
	while(*ptrInicial != NULL){
		ptrTemp = *ptrInicial;
		*ptrInicial = (*ptrInicial)->ptrSiguiente;	//avance
		free(ptrTemp);								//liberamos memoria
	}//fin del while
	return 0;	
}//fin de la funcion borrar_lista

float conversion(float volts, Sensor sens){
	//DESCRIPCION----------------------------------------------------------------------------------------------------------
	//Esta funcion convierte una entrada analogica de 10bits al tipo de magnitud del sensor (temp, presion, humedad, etc)
	//volts: entrada
	//sens: sensor transmisor

	//VARIABLES------------------------------------------------------------------------------------------------------------
	float salida;	//magnitud resultante de la conversion
	
	//CUERPO DE LA FUNCION-------------------------------------------------------------------------------------------------

	//tipo de curva lineal
	if (strcmp(sens.tipo_curva.tipo, "lineal") == 0) 
		salida = sens.tipo_curva.a * volts + sens.tipo_curva.b;
		
	//tipo de curva exponencial
	else if  (strcmp(sens.tipo_curva.tipo, "exp") == 0)  
		salida = exp(sens.tipo_curva.a * volts) + sens.tipo_curva.b;
	
	//tipo de curvq logaritmica
//	else if  (strcmp(sens.tipo_curva.tipo, "log") == 0)  
//		salida = log(volts)*sens.tipo_curva.a + sens.tipo_curva.b;
		
	//tipo de curva polinomica
	else if  (strcmp(sens.tipo_curva.tipo, "polinomica") == 0)  
		salida = sens.tipo_curva.a * sqrt(volts) + sens.tipo_curva.b * volts + sens.tipo_curva.c;
	
	//tipo de curva no reconocido	
	else
		return -1;		//la conversion no se realizo
		
	return salida;
}//fin de la funcion

void udp_cliente(int *sock, struct sockaddr_in *cliente, struct sockaddr_in *servidor){

	//1. Creamos un socket
	if ((*sock = socket(AF_INET, SOCK_DGRAM, 0)) == -1)
		printf("Socket no creado\n");

	//2. Cargamos nuestos datos como Clientes: todas las direcciones locales y cualquier numero de puerto
	memset((char *)cliente, 0, sizeof(*cliente));	 //seteamos la direccion en 0
	cliente->sin_family = AF_INET;					//domino
	cliente->sin_port = htons(0);					//cualquier puerto, ordenacion de la red 
	cliente->sin_addr.s_addr = htonl(INADDR_ANY);	//ordenacion de la red 	

	//3. Enlazamos el socket con nuestra direccion:
	if (bind(*sock, (struct sockaddr *)cliente, sizeof(*cliente)) < 0)
		perror("Error en el enlace\n");
		
	//3. Definimos la direccion del servidor
	memset((char *) servidor, 0, sizeof(*servidor));	//seteamos la direccion en 0
	servidor->sin_family = AF_INET;						//dominio
	servidor->sin_port = htons(SERVICE_PORT);			//puerto bien definido
	if (inet_aton(SERVER_IP, &servidor->sin_addr)==0) {		//convertimos la direccion IP de ascii a binario
		fprintf(stderr, "La funcion inet_aton() fallo\n");
		exit(1);
		}//fin del if

}//fin de la funcion

//FUNCIONES AUXILIARES-----------------------------------------------------------------------------------------------------

void ingresar_sensor(Sensor* ptrS, int num){
	//DESCRIPCION----------------------------------------------------------------------------------------------------------
	/*	Esta funcion almacena los datos ingresados por teclado dentro de una estructura "Sensor" pasado por referencia
	ptrS: puntero a la estructura "Sensor"
	num: opcional - permite asignar el numero de sensor antes de llamar la funcion, ingresar 0 para inhabilitar.	*/
	
	//VARIABLES------------------------------------------------------------------------------------------------------------
	int i;
	
	//CUERPO DE LA FUNCION-------------------------------------------------------------------------------------------------
	if(num != 0)
		ptrS->nro = num;
	else{
		printf("Ingrese nro de sensor: ");
		fflush(stdin);
		scanf("%d", &ptrS->nro);
		}//fin del else
	printf("Ingrese nombre de sensor: ");
		fflush(stdin);
		//fgets(ptrS->nombre, NOMLEN, stdin); 
		//ptrS->nombre[strlen(ptrS->nombre)-1] = '\0';	//quitamos el \n oculto
		scanf("\n%s", ptrS->nombre);
	printf("Ingrese tipo de curva del sensor: ");
		fflush(stdin);
		scanf("%s", ptrS->tipo_curva.tipo);
		for (i = 0; i < TYPELEN; i++)
			ptrS->tipo_curva.tipo [i] = tolower(ptrS->tipo_curva.tipo [i]);	//si esta en mayuscula lo paso a minuscula
	while(strcmp(ptrS->tipo_curva.tipo, "lineal") != 0 && strcmp(ptrS->tipo_curva.tipo, "log") != 0 
		&& strcmp(ptrS->tipo_curva.tipo, "exp") != 0 && strcmp(ptrS->tipo_curva.tipo, "polinomica") != 0){
		printf("Tipo de curva no reconocido\nIngrese nuevamente el tipo de curva del sensor: ");
		fflush(stdin);
		scanf("%s", ptrS->tipo_curva.tipo);
		for (i = 0; i < TYPELEN; i++)
			ptrS->tipo_curva.tipo [i] = tolower(ptrS->tipo_curva.tipo [i]);	//si esta en mayuscula lo paso a minuscula
		}//fin del while
	printf("Ingrese los tres parametros de la curva: ");
		scanf("%f %f %f", &ptrS->tipo_curva.a, &ptrS->tipo_curva.b, &ptrS->tipo_curva.c);	
//	ptrS->ptrSiguiente = NULL;
}//fin de la funcion agregar sensor

int insertar_sensor(Sensor** ptrInicial, Sensor* ptrNuevo){
	//DESCRIPCION----------------------------------------------------------------------------------------------------------
	//Esta funcion inserta un sensor nuevo en una lista de sensores
	//retorna -1 si esta ocupado y 0 si fue cargado
	
	//FUNCIONES AUXILIARES-------------------------------------------------------------------------------------------------
	Sensor* buscar_sensor(Sensor* ptrInicial, int num);
	
	//VARIABLES------------------------------------------------------------------------------------------------------------
	Sensor* ptrAnterior; 	//puntero a un nodo previo de la lista
	Sensor*  ptrActual; 	//puntero al nodo actual de la lista

	//CUERPO DE LA FUNCION-------------------------------------------------------------------------------------------------
	//revisamos que el numero de sensor no este ocupado
	if (buscar_sensor(*ptrInicial, ptrNuevo->nro) != NULL){	
		return -1;
		}//fin del if
	else{
	
	ptrAnterior = NULL;
	ptrActual = *ptrInicial;
	
	//ciclo para localizar la ubicación correcta en la lista
	while ( ptrActual != NULL && ptrNuevo->nro > ptrActual->nro ) {
	ptrAnterior = ptrActual; 
	ptrActual = ptrActual->ptrSiguiente;
	} //fin del while
	
	 // inserta el sensor al principio de la lista
	if ( ptrAnterior == NULL ) {
	ptrNuevo->ptrSiguiente = *ptrInicial;
	*ptrInicial = ptrNuevo;
	 } //fin del if
	 	 
	else { // inserta el sensor entre dos sensores de la lista
	ptrAnterior->ptrSiguiente = ptrNuevo;
	ptrNuevo->ptrSiguiente = ptrActual;
	} //fin del else
	return 0;
	}//fin del else
}//fin de la funcion

int eliminar_sensor(Sensor** ptrInicial, int num) {
	//DESCRIPCION----------------------------------------------------------------------------------------------------------
	//Esta funcion permite eliminar un sensor de la lista, retorna el numero borrado, 
	//retorna -1 si en la lista no hay sensores y -2 si no se encontro el sensor 
	//ptrInicial: puntero a la lista de sensores
	//num: numero de sensor

	//VARIABLES------------------------------------------------------------------------------------------------------------
	Sensor* ptrAnterior;
	Sensor* ptrActual; 
	Sensor* ptrTemp;
	
	//CUERPO DE LA FUNCION-------------------------------------------------------------------------------------------------
	if (*ptrInicial == NULL) {
		return -1;	//no hay sensores en la lista
		}//fin del if
	
	else {
		//elimina el primer sensor
		if ( num == ( *ptrInicial )->nro ) {
			ptrTemp = *ptrInicial; /* almacena el nodo a eliminar */
			*ptrInicial = ( *ptrInicial )->ptrSiguiente; /* desata el nodo */
			free( ptrTemp ); /* libera el nodo desatado */
			return num;
			} //fin del if
		
		//algoritmo de busqueda	
		else {
			ptrAnterior = *ptrInicial;
			ptrActual = ( *ptrInicial )->ptrSiguiente;
			
			// ciclo para localizar la ubicación correcta en la lista
			while ( ptrActual != NULL && ptrActual->nro != num ) {
			ptrAnterior = ptrActual; 
			ptrActual = ptrActual->ptrSiguiente;
			} //fin del while
			
			//elimina el sensor
			if ( ptrActual != NULL ) {
				ptrTemp = ptrActual;
				ptrAnterior->ptrSiguiente = ptrActual->ptrSiguiente;
				free( ptrTemp );
				return num;		//la funcion devuelve el numero del sensor recientemente eliminado
			} //fin del if
			
			//no encontro el sensor
			else 
				return -2;
		} //fin del else
	}//fin del else
} //fin de la funcion

Sensor* buscar_sensor(Sensor* ptrInicial, int num){
	//DESCRIPCION----------------------------------------------------------------------------------------------------------
	//Esta funcion permite busca un sensor de la lista, retorna un puntero al sensor o NULL si no se encuentra
	//ptrInicial: puntero a la lista de sensores
	//num: numero de sensor

	//VARIABLES------------------------------------------------------------------------------------------------------------
	Sensor* ptrActual; 
	
	//CUERPO DE LA FUNCION-------------------------------------------------------------------------------------------------
	//si la lista esta vacia
	if(ptrInicial == NULL)
		return NULL;

	//devuelve el puntero al primer sensor
	if ( num == ptrInicial->nro ) {
		return ptrInicial;
		} //fin del if
		
	else {
		ptrActual = ptrInicial->ptrSiguiente;
		// ciclo para localizar la ubicación del sensor
		while ( ptrActual != NULL && ptrActual->nro != num ) {
			ptrActual = ptrActual->ptrSiguiente;	//avance
			} //fin del while
		
		//si se encontro el sensor
		if ( ptrActual != NULL ) {
			return ptrActual;
			} //fin del if
		
		//si no se encontro el sensor
		else 
			return NULL;
	} //fin del else
} //fin de la funcion

int modificar_sensor(Sensor* ptrInicial, int num){
	//DESCRIPCION----------------------------------------------------------------------------------------------------------
	/*Esta funcion permite modificar los datos de un sensor, retorna -1 si la lista esta vacia y -2 si no se encontro el sensor
	ptrInicial: puntero a la lista de sensores
	num: numero de sensor	*/
	
	//FUNCIONES AUXILIARES-------------------------------------------------------------------------------------------------
	Sensor* buscar_sensor(Sensor* ptrInicial, int num);
	void ingresar_sensor(Sensor* ptrS, int num);
	
	//VARIABLES------------------------------------------------------------------------------------------------------------
	Sensor* ptrSensor;
	
	//CUERPO DE LA FUNCION-------------------------------------------------------------------------------------------------
	//Lista vacia
	if (ptrInicial == NULL) {
	return -1;	//no hay sensores cargados en la lista
	}//fin del if
	
	else {
		ptrSensor = buscar_sensor(ptrInicial, num);
		if(ptrSensor == NULL){
			return -2;	//sensor no encontrado
			}//fin del if
		else{
			ingresar_sensor(ptrSensor, num);
			}//fin del else
		return 0;
	}//fin del else
}//fin de la funcion

int importar_sensor(FILE* ptrF, Sensor** ptrInicial, int nro_sensor){
	//DESCRIPCION----------------------------------------------------------------------------------------------------------
	/*Importa y agrega a la lista un sensor desde un archivo, 
	retorna 0 si fue cargado, -1 si el sensor esta existente, -2 si no se encontro y -3 memoria insuficiente.-
	ptrArchivo: puntero al archivo registro de configuracion de sensores
	ptrInicial: puntero a la lista de sensores
	int numero_sensor: numero del sensor buscado	*/
	
	//FUNCIONES AUXILIARES-------------------------------------------------------------------------------------------------
	int insertar_sensor(Sensor** ptrInicial, Sensor* ptrNuevo);
	Sensor* buscar_sensor(Sensor* ptrInicial, int num);
	
	//VARIABLES------------------------------------------------------------------------------------------------------------
	Sensor nodo_temp = {0};	//nodo temporal (guardo los datos de cada barrido)
	Sensor* ptrNuevo;	//nodo nuevo y permanente
	
	//CUERPO DE LA FUNCION-------------------------------------------------------------------------------------------------
	while (!feof(ptrF)) {
			//guardamos los datos obtenidos del archivo en un nodo temporal
			fscanf(ptrF,"Numero de Sensor: %d\nNombre de Sensor: %s\nCurva: %s - Parametros A: %f B: %f C: %f\n"
			"-----------------------------------------------------\n", 
			&nodo_temp.nro, nodo_temp.nombre, nodo_temp.tipo_curva.tipo, 
			&nodo_temp.tipo_curva.a, &nodo_temp.tipo_curva.b, &nodo_temp.tipo_curva.c);
			nodo_temp.ptrSiguiente = NULL;
				
			//sensor encontrado		
			if (nodo_temp.nro == nro_sensor) {
				ptrNuevo = (Sensor *)malloc(sizeof(Sensor));	//pido memoria
		
				if (ptrNuevo == NULL) 
				return -3;	//memoria insuficiente
				
				else{						//Memoria asignada
					*ptrNuevo = nodo_temp;
					//insertamos el sensor obtenido en la lista de sensores
					if(insertar_sensor(ptrInicial, ptrNuevo) < 0)
						return -1;		//sensor existente
					else
						return 0;		//sensor cargado
					}//fin memoria asignada
				}//fin de sensor encontrado
				
			//sensor parcialmente no encontrado, seguimos recorriendo el archivo
			memset(&nodo_temp, 0, sizeof(nodo_temp));	//seteamos en 0 el nodo temporal
			
		}//fin del recorrido del archivo
	
	fclose(ptrF);	//cerramos el archivo
	
	//sensor no entontrado en el archivo
	if (nodo_temp.nro != nro_sensor)
		return -2;	//sensor no encontrado
	return 0;
}//fin de la funcion

float simulador(){
	//DESCRIPCION-----------------------------------------------------------------------------------------
	/*Esta funcion simula la señal de entrada analogica de un sensor.-	*/
	
	//VARIABLES-------------------------------------------------------------------------------------------
	clock_t reloj;
	
	//CUERPO DE LA FUNCION--------------------------------------------------------------------------------
	sleep(SAMPLE_RATE);
	reloj = clock();
	return log(reloj);
	
}//fin de la funcion

int leer_sensores(Sensor* ptrLee, char** transmision){
	//DESCRIPCION----------------------------------------------------------------------------------------------------------
	/*Esta funcion escanea las mediciones en volts de todos los sensores, los convierte en la magnitud mensurada en funcion de 
	la curva de cada sensor, y luego guarda los datos en una cadena, retorna -1 si la lista de sensores esta vacia.-
	ptrLee: puntero a la lista de sensores.-	*/
	 
	//VARIABLES------------------------------------------------------------------------------------------------------------
	float medicion;
	char buffer_transmision[BUFSIZE] = {0};	//buffer de transmision	
	char buffer_medicion[BUFSIZE] = {0};	//buffer de medicion
	
	//CUERPO DE LA FUNCION-------------------------------------------------------------------------------------------------
	//lista vacia
	if(ptrLee == NULL)
		return -1;
	
	// Paneo de sensores
	while (ptrLee != NULL){
		//lectura
		medicion = conversion(simulador(), *ptrLee);
		//transmision	
		snprintf(buffer_medicion, sizeof (buffer_medicion),  
		"\"S_%d_%d\" : %.2f,", RPI_ID, ptrLee->nro, medicion);		//formato
		strcat(buffer_transmision, buffer_medicion);				//cadena de transmision
		buffer_medicion[0] = '\0'; 	//limpio buffer													
		//siguiente sensor																  
		ptrLee = ptrLee->ptrSiguiente;
	}//fin del paneo
	
	*transmision = (char *)malloc(strlen(buffer_transmision));
	strcpy(*transmision, buffer_transmision);
	buffer_transmision[0] = '\0'; 	//limpio buffer	
	return 0;
}//fin de la funcion

int imprime_nodo(Sensor* ptrNodo){
	if(ptrNodo == NULL)
		return -1;
	else{
		printf("\n-----------------------------------------------------\n");
		fprintf(stdout,"Numero de Sensor: %d\nNombre de Sensor: %s\nCurva: %s - Parametros A: %.2f B: %.2f C: %.2f\n"
		"-----------------------------------------------------\n", 
		ptrNodo->nro, ptrNodo->nombre, ptrNodo->tipo_curva.tipo, 
		ptrNodo->tipo_curva.a, ptrNodo->tipo_curva.b, ptrNodo->tipo_curva.c);
		return 0;
	}//fin del else
}//fin de la funcion

void registro(FILE *archivo_log, char *texto){
	//DESCRIPCION----------------------------------------------------------------------------------------------------------
	//Esta funcion registra un evento con fecha y hora en un archivo registro
	
	//VARIABLES------------------------------------------------------------------------------------------------------------
	time_t tiempo = time(0);	//me devuelve un long con los segundos transcurridos desde 01/01/1970
    struct tm *tlocal = localtime(&tiempo);		//transforma los segundos en dias, meses, años, horas, minutos y segundos adaptada a nuestra zona horaria
    char output[128];	
    
	//CUERPO DE LA FUNCION-------------------------------------------------------------------------------------------------
    strftime(output,128,"%d/%m/%y %H:%M:%S",tlocal);
    fprintf(archivo_log, "%s - %s\n",output, texto);
    
}//fin de la funcion registro

void imprime(char* texto){
	printf("\n-----------------------------------------------------\n***%s"
	"***\n-----------------------------------------------------\n", texto);
}//fin de funcion imprime


void put_to_send(char str_base[], char str_recv[]){
	//subfunciones
	void cargar_str(char* str_base, char *str_transm);
	
	//variables
	char *substr_recv = NULL;

	//dividimos el str_recv en varios substr_recv
	substr_recv = strtok(str_recv, ",");
	while (substr_recv != NULL) {
	//	printf("%s\n", substr_recv);
		cargar_str(str_base, substr_recv);
		substr_recv = strtok(NULL, ",");
		if(substr_recv != NULL)	substr_recv = &substr_recv[1];
		}//fin del while
}//fin de la funcion put_to_send

void cargar_str(char* str_base, char *substr_recv){
	//subfunciones
	char *reemplazar_substr(char *str, char *substr_orig, char *substr_remp);
	
	//variables
	char cabecera[20] = {0};
	char *loc = NULL;
//	printf("substring recibido %s\n", substr_recv );
	//cabecera
	strncpy(cabecera, substr_recv, 7);	
//	printf("cabecera: %s\n", cabecera);
	
	//substr_original
	loc = strstr(str_base, cabecera);
	snprintf(cabecera, strchr(loc, ',') - loc, "%s\n", loc);	
//	printf("substring original %s\n", cabecera );
	//modificamos el str_base
	strcpy(str_base, reemplazar_substr(str_base, cabecera, substr_recv));
	
}//fin de la funcion cargar_str

char *reemplazar_substr(char *str, char *substr_orig, char *substr_remp){
  	//variables
	static char buffer[4096];
  	char *loc = NULL;
	
	//localizazion del substring original
  	if(!(loc = strstr(str, substr_orig)))  
    return str;

  	strncpy(buffer, str, loc-str); // copia caracteres desde str hasta substr_orig
  	buffer[loc-str] = '\0';
	
	//str con el substring reemplazado
  	sprintf(buffer+(loc-str), "%s%s", substr_remp, loc+strlen(substr_orig));

  	return buffer;
}//fin de la funcion reemplazar_substr


	//DESCRIPCION----------------------------------------------------------------------------------------------------------
	
	//FUNCIONES AUXILIARES-------------------------------------------------------------------------------------------------
	
	//VARIABLES------------------------------------------------------------------------------------------------------------
	
	//CUERPO DE LA FUNCION-------------------------------------------------------------------------------------------------

