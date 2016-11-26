/*
VERSIONES: 
v.1- incorporacion de archivo log y funcion registro.-
v.2- incorporacion de funcion imprime, optimizacion de la transmision		
v.3 - deteccion de archivo vacio	*/

//---------------------------------------------------------------------------------------------------------------------------------
#include <stdio.h>
#include"sensorlib.h"
#define ARCHIVO_SENSORES "sensores.txt"
#define ARCHIVO_LOG "log.txt"
#define LOG_SIZE 100
#define PRINT_SIZE 100

//ENCABEZADOS----------------------------------------------------------------------------------------------------------------------	
int menu_sensor();
void imprime(char* texto);

int main(){
	
	//VARIABLES--------------------------------------------------------------------------------------------------------------------
	FILE* archivo, *log;
	int eleccion, numero, contador = 0;
	char *cadena, mensaje_log[LOG_SIZE], mensaje_pantalla[PRINT_SIZE];
	Sensor* ptrInicial = NULL;
	
	//CUERPO DEL PROGRAMA----------------------------------------------------------------------------------------------------------
	log = fopen(ARCHIVO_LOG, "w+");
	if (log == NULL){
	imprime("NO PUDO ABRIRSE EL ARCHIVO DE REGISTRO LOG");
	return -1;
	}//fin del if
		
	registro(log, "Iniciando Configuracion de Sensores");
	eleccion = menu_sensor();
	
	while (eleccion != 10){
		mensaje_log[0] = '\0';
		mensaje_pantalla[0] = '\0';
		switch(eleccion){
			case 1:		//imprimir lista de sensores
				if (imprime_sensores(ptrInicial) < 0)
				imprime("NO HAY SENSORES CARGADOS");	
				break;
			case 2:		//Cargar sensores del archivo de configuracion
				archivo = fopen(ARCHIVO_SENSORES, "r");
				if(archivo == NULL){
					imprime("NO SE PUDO ABRIR EL ARCHIVO DE CONFIGURACION");
					snprintf(mensaje_log, LOG_SIZE, "No se pudo abrir el archivo de configuracion \"%s\"", ARCHIVO_SENSORES);
					registro(log, mensaje_log);
					}//fin del if
				else{
					switch (importar_sensores(archivo, &ptrInicial)) {
					case -1:
						imprime("ARCHIVO DE CONFIGURACION VACIO");
						registro(log, "Archivo de configuracion vacio");
					break;
					case -2:
						imprime("MEMORIA INSUFICIENTE");
						registro(log, "Memoria insuficiente");
					break;
					default:	
						imprime("SENSORES CARGADOS SATISFACTORIAMENTE");
						snprintf(mensaje_log, LOG_SIZE, "Sensores cargados satisfactoriamente desde el archivo \"%s\"", ARCHIVO_SENSORES);
						registro(log, mensaje_log);
					break;
						}//fin del  switch
					}//fin del else
				break;
			case 3: 	//Buscar y cargar un sensor desde el archivo de configuracion 
				archivo = fopen(ARCHIVO_SENSORES, "r");
				if(archivo == NULL){
					snprintf(mensaje_pantalla, PRINT_SIZE, "NO SE PUDO ABRIR EL ARCHIVO DE CONFIGURACION \"%s\"", ARCHIVO_SENSORES);
					imprime(mensaje_pantalla);
					snprintf(mensaje_log, LOG_SIZE, "No se pudo abrir el archivo de configuracion \"%s\"", ARCHIVO_SENSORES);
					registro(log, mensaje_log);
					}//fin del if
				else{
					printf("\nIngrese el numero de sensor: ");
					scanf("\n%d", &numero);	
					switch (importar_sensor(archivo, &ptrInicial, numero)) {
						case -1:
							snprintf(mensaje_pantalla, PRINT_SIZE, "SENSOR \"%d\" EXISTENTE", numero); 
							imprime(mensaje_pantalla);
							break;
						case -2:
							snprintf(mensaje_pantalla, PRINT_SIZE, "SENSOR \"%d\" NO ENCONTRADO", numero); 
							imprime(mensaje_pantalla);
							break;
						case -3:
							imprime("MEMORIA INSUFICIENTE");
							registro(log, "Memoria insuficiente");				
							break;
						default:
							snprintf(mensaje_pantalla, PRINT_SIZE, "SENSOR \"%d\" CARGADO", numero); 
							imprime(mensaje_pantalla);
							snprintf(mensaje_log, LOG_SIZE, "Sensor \"%d\" cargado", numero);
							registro(log, mensaje_log);
							break;
						}//fin del switch
					}//fin del else			
				break;
			case 4:		//Ingresar sensor manualmente
				if(ingresar_sensores(&ptrInicial) < 0){
				imprime("MEMORIA INSUFICIENTE");
				registro(log, "Memoria insuficiente");
				}//fin del if
				break;
			case 5: 	//Borrar sensor
				printf("Ingrese el numero de sensor: ");
				scanf("\n%d", &numero);
				switch(eliminar_sensor(&ptrInicial, numero)){
					case -1:
						imprime("NO HAY SENSORES CARGADOS");
						break;
					case -2:
						snprintf(mensaje_pantalla, PRINT_SIZE, "SENSOR \"%d\" NO ENCONTRADO",numero); 
						imprime(mensaje_pantalla);
						break;
					default:
						snprintf(mensaje_pantalla, PRINT_SIZE, "SENSOR \"%d\" BORRADO",numero); 
						imprime(mensaje_pantalla);
						snprintf(mensaje_log, LOG_SIZE, "Sensor \"%d\" borrado", numero);
						registro(log, mensaje_log);
						break;
					}//fin del switch
				break;
			case 6:	//Modificar sensor
				printf("Ingrese el numero de sensor: ");
				scanf("\n%d", &numero);
				switch(modificar_sensor(ptrInicial, numero)){
					case -1:
						imprime("NO HAY SENSORES CARGADOS");
						break;
					case -2:
						snprintf(mensaje_pantalla, PRINT_SIZE, "SENSOR \"%d\" NO ENCONTRADO",numero); 
						imprime(mensaje_pantalla);
						break;
					default:
						snprintf(mensaje_pantalla, PRINT_SIZE, "SENSOR \"%d\" MODIFICADO",numero); 
						imprime(mensaje_pantalla);
						snprintf(mensaje_log, LOG_SIZE, "Sensor \"%d\" modificado", numero);
						registro(log, mensaje_log);
					break;
				}//fin del switch
				break;
			case 7:		//borrar lista de sensores
				if(borrar_lista(&ptrInicial) == 0){
				imprime("SENSORES BORRADOS DE LA LISTA");
				registro(log, "Sensores borrados de la lista");
					}//fin del if
				break;								
			case 8:		//Guardar lista de sensores en el archivo de configuracion
				archivo = fopen(ARCHIVO_SENSORES, "w");
				if (archivo == NULL) {
				imprime("MEMORIA INSUFICIENTE");
				registro(log, "Memoria insuficiente");
					}//fin del if
				exportar_sensores(archivo, ptrInicial);
				snprintf(mensaje_pantalla, PRINT_SIZE, "SENSORES GUARDADOS EN EL ARCHIVO \"%s\"", ARCHIVO_SENSORES); 
				imprime(mensaje_pantalla);
				snprintf(mensaje_log, LOG_SIZE, "Sensores guardados en el archivo \"%s\"", ARCHIVO_SENSORES);
				registro(log, mensaje_log);
				break;
			case 9:	//Ver modelo de transmision de datos
				while (contador < 4){
					if(leer_sensores(ptrInicial, &cadena) < 0){	//devuelve -1 si la lista esta vacia
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
	registro(log, "Configuracion Finalizada");
	fclose(log);
	return 0;
}//fin del main

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
	"Opcion ingresada: ", ARCHIVO_SENSORES, ARCHIVO_SENSORES, ARCHIVO_SENSORES);
	scanf("%d", &eleccion);
	return eleccion;
}//fin del menu




