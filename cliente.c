/*
PROBADO EL SABADO 19 DE NOVIEMBRE - transmite al server
v.1 - Incorporacion de la funcion configuracion_sensores.
v.2 - Retardo al inicial Configuracion de Sensores y deteccion de archivo de configuracion vacio
* 
*    gcc -o cliente1 cliente.c sensorlib.h -lm
* 
*/

#include"sensorlib.h"
#define ARCHIVO "sensores.txt"
#define MAX_SENSORES 4			    							
#define BUFLEN 2048
#define ARCHIVO_LOG "log.txt"
#define LOG_SIZE 100
#define PRINT_SIZE 100
#define TIME_CONF 1

int configuracion_sensores(FILE **archivo, FILE **log, Sensor **ptrSensores);
int menu_sensor();
void imprime(char* texto);
//int kbhit(void);

int main(){

	//variables								
	FILE *archivo = NULL, *log = NULL;
	Sensor *ptrInicial = NULL;
	char *transmision, texto_log[LOG_SIZE];
	struct sockaddr_in cliente, servidor;	
	int descriptor, slen = sizeof(servidor);
	//int contador = 0; 
	/*
	//Retardo para ingresar a Configuracion de Sensores
	printf("Presione una tecla para ingresar a \"Configuracion de Sensores\" ");
	while( !kbhit() && contador < 5){
	sleep(TIME_CONF);
	printf("*");
	contador++;
	}//fin del while
		
	//ingresando a Configuracion de Sensores
	if(contador < 5)
	configuracion_sensores(&archivo, &log, &ptrInicial);*/
	
	//inicializando Archivo LOG
	imprime("Inicializamos Archivo Log");
	log = fopen(ARCHIVO_LOG, "w+");
	if (log == NULL){
	imprime("NO PUDO ABRIRSE EL ARCHIVO DE REGISTRO LOG");
	return -1;
	}//fin del if
	
	//inicializando Archivo de Configuracion
	imprime("Inicializamos Archivo de Configuracion");
	registro(log, "Inicializando Archivo de Configuracion");
	archivo = fopen(ARCHIVO, "r");
	if (archivo == NULL){
	imprime("NO SE PUDO ABRIR EL ARCHIVO DE CONFIGURACION");
	snprintf(texto_log, LOG_SIZE, "No se pudo abrir el archivo de configuracion \"%s\"", ARCHIVO);
	registro(log, texto_log);
	fclose(log);
	return -2;
	}//fin del if
	
	//cargamos sensores
	imprime("Cargamos Sensores");
	registro(log, "Cargando Sensores");	
	switch (importar_sensores(archivo, &ptrInicial)) {
	case -1:
		imprime("ARCHIVO DE CONFIGURACION VACIO");
		registro(log, "Archivo de configuracion vacio");
		fclose(log);
		return 0;
		break;
	case -2:
		imprime("MEMORIA INSUFICIENTE");
		registro(log, "Memoria insuficiente");
		fclose(log);
		return 0;
		break;
	default:	
		imprime("Sensores CArgados Satisfactoriamente");
		snprintf(texto_log, LOG_SIZE, "Sensores cargados satisfactoriamente desde el archivo \"%s\"", ARCHIVO);
		registro(log, texto_log);
		break;
	}//fin del  switch

	imprime("Imprimimos sensores cargados");
	imprime_sensores(ptrInicial);

	//inicializacion de socket UDP
	udp_cliente(&descriptor, &cliente, &servidor);

	//transmitimos las mediciones
	imprime("Transmitiendo Mediciones");
	while (1) {
		if(leer_sensores(ptrInicial, &transmision) < 0){	//devuelve -1 si la lista esta vacia
			imprime("NO HAY SENSORES CARGADOS");
			break;
			}//fin del while	
		else
			printf("transmitiendo: %s", transmision);
			if (sendto(descriptor, transmision, strlen(transmision), 0, (struct sockaddr *)&servidor, slen) == -1) {
			registro(log, "Error al enviar el mensaje por socket\n");
			return -1;
			}//fin del if
	}//fin del while
	return 0;
}//fin del main


int configuracion_sensores(FILE **archivo, FILE **log, Sensor **ptrSensores){
	
	//VARIABLES------------------------------------------------------------------------------------------------------------
	int eleccion, numero, contador = 0;
	char *cadena, mensaje_log[LOG_SIZE], mensaje_imprime[PRINT_SIZE];
	
	//CUERPO DE LA FUNCION-------------------------------------------------------------------------------------------------
	registro(*log, "Iniciando Configuracion de Sensores");
	eleccion = menu_sensor();
	
	while (eleccion != 10){
		mensaje_log[0] = '\0';
		mensaje_imprime[0] = '\0';
		switch(eleccion){
			case 1:		//imprimir lista de sensores
				if (imprime_sensores(*ptrSensores) < 0)
				imprime("NO HAY SENSORES CARGADOS");	
				break;
			case 2:		//Cargar sensores del archivo de configuracion
				*archivo = fopen(ARCHIVO, "r");
				if(*archivo == NULL){
					imprime("NO SE PUDO ABRIR EL ARCHIVO DE CONFIGURACION");
					snprintf(mensaje_log, LOG_SIZE, "No se pudo abrir el archivo de configuracion \"%s\"", ARCHIVO);
					registro(*log, mensaje_log);
					}//fin del if
				else{
					switch (importar_sensores(*archivo, ptrSensores)) {
					case -1:
						imprime("ARCHIVO DE CONFIGURACION VACIO");
						registro(*log, "Archivo de configuracion vacio");
					break;
					case -2:
						imprime("MEMORIA INSUFICIENTE");
						registro(*log, "Memoria insuficiente");
					break;
					default:	
						imprime("SENSORES CARGADOS SATISFACTORIAMENTE");
						snprintf(mensaje_log, LOG_SIZE, "Sensores cargados satisfactoriamente desde el archivo \"%s\"", ARCHIVO);
						registro(*log, mensaje_log);
					break;
						}//fin del  switch
					}//fin del else
				break;
			case 3: 	//Buscar y cargar un sensor desde el archivo de configuracion 
				*archivo = fopen(ARCHIVO, "r");
				if(*archivo == NULL){
					snprintf(mensaje_imprime, PRINT_SIZE, "NO SE PUDO ABRIR EL ARCHIVO DE CONFIGURACION \"%s\"", ARCHIVO);
					imprime(mensaje_imprime);
					snprintf(mensaje_log, LOG_SIZE, "No se pudo abrir el archivo de configuracion \"%s\"", ARCHIVO);
					registro(*log, mensaje_log);
					}//fin del if
				else{
					printf("\nIngrese el numero de sensor: ");
					scanf("\n%d", &numero);	
					switch(importar_sensor(*archivo, ptrSensores, numero)){
						case -1:
							snprintf(mensaje_imprime, PRINT_SIZE, "SENSOR \"%d\" EXISTENTE", numero); 
							imprime(mensaje_imprime);
							break;
						case -2:
							snprintf(mensaje_imprime, PRINT_SIZE, "SENSOR \"%d\" NO ENCONTRADO", numero); 
							imprime(mensaje_imprime);
							break;
						case -3:
							imprime("MEMORIA INSUFICIENTE");
							registro(*log, "Memoria insuficiente");				
							break;
						default:
							snprintf(mensaje_imprime, PRINT_SIZE, "SENSOR \"%d\" CARGADO", numero); 
							imprime(mensaje_imprime);
							snprintf(mensaje_log, LOG_SIZE, "Sensor \"%d\" cargado", numero);
							registro(*log, mensaje_log);
							break;
						}//fin del switch
					}//fin del else			
				break;
			case 4:		//Ingresar sensor manualmente
				if(ingresar_sensores(ptrSensores) < 0){
				imprime("MEMORIA INSUFICIENTE");
				registro(*log, "Memoria insuficiente");
				}//fin del if
				break;
			case 5: 	//Borrar sensor
				printf("Ingrese el numero de sensor: ");
				scanf("\n%d", &numero);
				switch(eliminar_sensor(ptrSensores, numero)){
					case -1:
						imprime("NO HAY SENSORES CARGADOS");
						break;
					case -2:
						snprintf(mensaje_imprime, PRINT_SIZE, "SENSOR \"%d\" NO ENCONTRADO",numero); 
						imprime(mensaje_imprime);
						break;
					default:
						snprintf(mensaje_imprime, PRINT_SIZE, "SENSOR \"%d\" BORRADO",numero); 
						imprime(mensaje_imprime);
						snprintf(mensaje_log, LOG_SIZE, "Sensor \"%d\" borrado", numero);
						registro(*log, mensaje_log);
						break;
					}//fin del switch
				break;
			case 6:	//Modificar sensor
				printf("Ingrese el numero de sensor: ");
				scanf("\n%d", &numero);
				switch(modificar_sensor(*ptrSensores, numero)){
					case -1:
						imprime("NO HAY SENSORES CARGADOS");
						break;
					case -2:
						snprintf(mensaje_imprime, PRINT_SIZE, "SENSOR \"%d\" NO ENCONTRADO",numero); 
						imprime(mensaje_imprime);
						break;
					default:
						snprintf(mensaje_imprime, PRINT_SIZE, "SENSOR \"%d\" MODIFICADO",numero); 
						imprime(mensaje_imprime);
						snprintf(mensaje_log, LOG_SIZE, "Sensor \"%d\" modificado", numero);
						registro(*log, mensaje_log);
					break;
				}//fin del switch
				break;
			case 7:		//borrar lista de sensores
				if(borrar_lista(ptrSensores) == 0){
				imprime("SENSORES BORRADOS DE LA LISTA");
				registro(*log, "Sensores borrados de la lista");
					}//fin del if
				break;								
			case 8:		//Guardar lista de sensores en el archivo de configuracion
				*archivo = fopen(ARCHIVO, "w");
				if (*archivo == NULL) {
				imprime("MEMORIA INSUFICIENTE");
				registro(*log, "Memoria insuficiente");
					}//fin del if
				exportar_sensores(*archivo, *ptrSensores);
				snprintf(mensaje_imprime, PRINT_SIZE, "SENSORES GUARDADOS EN EL ARCHIVO \"%s\"", ARCHIVO); 
				imprime(mensaje_imprime);
				snprintf(mensaje_log, LOG_SIZE, "Sensores guardados en el archivo \"%s\"", ARCHIVO);
				registro(*log, mensaje_log);
				break;
			case 9:	//Ver modelo de transmision de datos
				while (contador < 10){
					if(leer_sensores(*ptrSensores, &cadena) < 0){	//devuelve -1 si la lista esta vacia
						imprime("NO HAY SENSORES CARGADOS");
						break;
						}//fin del while	
					else
						printf("%s\n", cadena);
						contador++;
					}//fin del while
				break;
			default:
				imprime("OPCION INVALIDA");
				break;
		}//fin del switch	
		eleccion = menu_sensor();
		}//fin del while
	registro(*log, "Configuracion Finalizada");
	fclose(*log);
	return 0;
}

int menu_sensor(){
	int eleccion;
	printf("\nCONFIGURACION DE SENSORES:\n\n"
	"1. Imprimir lista de sensores.\n"
	"2. Cargar sensores del archivo de configuracion \"%s\".\n"	
	"3. Buscar y cargar un sensor desde el archivo de configuracion \"%s\".\n"	
	"4. Ingresar sensor manualmente.\n"				
	"5. Borrar sensor.\n"
	"6. Modificar sensor.\n"	
	"7. Borrar lista de sensores.\n"
	"8. Guardar lista de sensores en el archivo de configuracion \"%s\". \n"	
	"9. Ver modelo de transmision de datos\n"						
	"10. Terminar ejecucion.\n"
	"Opcion ingresada: ", ARCHIVO, ARCHIVO, ARCHIVO);
	scanf("%d", &eleccion);
	return eleccion;
}//fin del menu
 /*
int kbhit(void)
{
  struct termios oldt, newt;
  int ch;
  int oldf;
 
  tcgetattr(STDIN_FILENO, &oldt);
  newt = oldt;
  newt.c_lflag &= ~(ICANON | ECHO);
  tcsetattr(STDIN_FILENO, TCSANOW, &newt);
  oldf = fcntl(STDIN_FILENO, F_GETFL, 0);
  fcntl(STDIN_FILENO, F_SETFL, oldf | O_NONBLOCK);
 
  ch = getchar();
 
  tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
  fcntl(STDIN_FILENO, F_SETFL, oldf);
 
  if(ch != EOF)
  {
    ungetc(ch, stdin);
    return 1;
  }
 
  return 0;
}*/
