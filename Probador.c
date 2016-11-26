/*
VERSIONES: 
v1- incorporacion de archivo log y funcion registro. incorporacion de funcion imprime, optimizacion de la transmision			
v2. incorporacion del formato de transmision a la nube*/

//---------------------------------------------------------------------------------------------------------------------------------
#include <stdio.h>
#include"sensorlib2.h"
#define ARCHIVO_SENSORES "sensores.txt"
#define ARCHIVO_LOG "log.txt"
#define LOG_SIZE 100
#define PRINT_SIZE 100
#define TIMESLEEP 1
#define TIMEPRINT 1500

//ENCABEZADOS----------------------------------------------------------------------------------------------------------------------	
int menu_sensor();
void imprime(char* texto);

int main(){
	
	//VARIABLES--------------------------------------------------------------------------------------------------------------------
	FILE* archivo, *log;
	int eleccion, numero, contador = 0;
	char *cadena, mensaje_log[LOG_SIZE], mensaje_imprime[PRINT_SIZE];
	Sensor* ptrCabeza = NULL, *nuevoSensor = NULL;
	time_t t_inicial;
	time_t t_final;
	char str_base[] =  "\"S_1_1\" : 0, \"S_1_2\" : 0, \"S_1_3\" : 0, \"S_2_1\" : 0, \"S_2_2\" : 0, \"S_2_3\" : 0, \"S_3_1\" : 0, \"S_3_2\" : 0, \"S_3_3\" : 0, \"S_3_4\" : 0, \"S_3_5\" : 0,";                       
	char str_transm[strlen(str_base)];
	char str_recv[100] = {0};
	
	//CUERPO DEL PROGRAMA----------------------------------------------------------------------------------------------------------
	log = fopen(ARCHIVO_LOG, "w+");
	if (log == NULL){
	imprime("NO PUDO ABRIRSE EL ARCHIVO DE REGISTRO LOG");
	return -1;
	}//fin del if
		
	registro(log, "Iniciando Configuracion de Sensores");
	eleccion = menu_sensor();
	
	while (eleccion != 13){
		mensaje_log[0] = '\0';
		mensaje_imprime[0] = '\0';
		switch(eleccion){
			case 1:		//imprimir lista de sensores
				if (imprime_sensores(ptrCabeza) < 0)
				imprime("NO HAY SENSORES CARGADOS");	
				break;
			case 2:		//Ingresar sensores por teclado
				if(ingresar_sensores(&ptrCabeza) < 0){
				imprime("MEMORIA INSUFICIENTE");
				registro(log, "Memoria insuficiente");
				}//fin del if
				break;
			case 3:		//Importar sensores
				archivo = fopen(ARCHIVO_SENSORES, "r");
				if(archivo == NULL){
					imprime("NO SE PUDO ABRIR EL ARCHIVO DE CONFIGURACION");
					snprintf(mensaje_log, LOG_SIZE, "No se pudo abrir el archivo de configuracion \"%s\"", ARCHIVO_SENSORES);
					registro(log, mensaje_log);
					}//fin del if
				else{
					if(importar_sensores(archivo, &ptrCabeza) < 0){
					imprime("MEMORIA INSUFICIENTE");
					registro(log, "Memoria insuficiente");
					}//fin del if		
					else
					imprime("SENSORES CARGADOS SATISFACTORIAMENTE");
					snprintf(mensaje_log, LOG_SIZE, "Sensores cargados satisfactoriamente desde el archivo \"%s\"", ARCHIVO_SENSORES);
					registro(log, mensaje_log);
					}//fin del else
				break;
			case 4: 	//importar un sensor
				archivo = fopen(ARCHIVO_SENSORES, "r");
				if(archivo == NULL){
					snprintf(mensaje_imprime, PRINT_SIZE, "NO SE PUDO ABRIR EL ARCHIVO DE CONFIGURACION \"%s\"", ARCHIVO_SENSORES);
					imprime(mensaje_imprime);
					snprintf(mensaje_log, LOG_SIZE, "No se pudo abrir el archivo de configuracion \"%s\"", ARCHIVO_SENSORES);
					registro(log, mensaje_log);
					}//fin del if
				else{
					printf("\nIngrese el numero de sensor: ");
					scanf("\n%d", &numero);	
					switch(importar_sensor(archivo, &ptrCabeza, numero)){
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
							registro(log, "Memoria insuficiente");				
							break;
						default:
							snprintf(mensaje_imprime, PRINT_SIZE, "SENSOR \"%d\" CARGADO", numero); 
							imprime(mensaje_imprime);
							snprintf(mensaje_log, LOG_SIZE, "Sensor \"%d\" cargado", numero);
							registro(log, mensaje_log);
							break;
						}//fin del switch
					}//fin del else			
				break;
			case 5:		//exportar configuracion de sensores
				archivo = fopen(ARCHIVO_SENSORES, "w");
				if (archivo == NULL) {
				imprime("MEMORIA INSUFICIENTE");
				registro(log, "Memoria insuficiente");
					}//fin del if
				exportar_sensores(archivo, ptrCabeza);
				snprintf(mensaje_imprime, PRINT_SIZE, "SENSORES GUARDADOS EN EL ARCHIVO \"%s\"", ARCHIVO_SENSORES); 
				imprime(mensaje_imprime);
				snprintf(mensaje_log, LOG_SIZE, "Sensores guardados en el archivo \"%s\"", ARCHIVO_SENSORES);
				registro(log, mensaje_log);
				break;
			case 6:		//borrar lista
				if(borrar_lista(&ptrCabeza) == 0){
				imprime("SENSORES BORRADOS DE LA LISTA");
				registro(log, "Sensores borrados de la lista");
					}//fin del if
				break;		
			case 7: 	//Borrar sensor
				printf("Ingrese el numero de sensor: ");
				scanf("\n%d", &numero);
				switch(eliminar_sensor(&ptrCabeza, numero)){
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
						registro(log, mensaje_log);
						break;
					}//fin del switch
				break;
			case 8:	//Modificar sensor
				printf("Ingrese el numero de sensor: ");
				scanf("\n%d", &numero);
				switch(modificar_sensor(ptrCabeza, numero)){
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
						registro(log, mensaje_log);
					break;
				}//fin del switch
				break;
			case 9:		//simulacion
				while(1)
				printf("Entrada de simulador: %.2f volts\n", simulador());
				break;	
			case 10:	//Transmision de mediciones
				t_inicial = clock();
				strcpy(str_transm, str_base);
				while (contador < 10){
					if(leer_sensores(ptrCabeza, &cadena) < 0){	//devuelve -1 si la lista esta vacia
						imprime("NO HAY SENSORES CARGADOS");
						break;
						}//fin del if
					str_recv[0] = '\0';
					strcpy(str_recv, cadena);
					printf("%s (%d)\n", str_recv);
					put_to_send(str_transm, str_recv);
					t_final = clock();		
					if ( (t_final - t_inicial) > TIMEPRINT ) {
						printf("envio a la nube\n");
						printf("%s\n", str_transm);
						str_transm[0] = '\0';
						strcpy(str_transm, str_base);
						t_inicial = clock();
						}//fin del if
						contador++;
					}//fin del while
				break;
				
			case 11:	//ingresamos un sensor
				if ((nuevoSensor = (Sensor *)malloc(sizeof(Sensor))) < 0) {
				imprime("MEMORIA INSUFICIENTE");
				registro(log, "Memoria insuficiente");
				break;
					}//fin del if
				ingresar_sensor(nuevoSensor, 0);			//cargamos los datos
				insertar_sensor(&ptrCabeza, nuevoSensor);	//insertamos en la lista
				snprintf(mensaje_imprime, PRINT_SIZE, "SENSOR \"%d\" INGRESADO", nuevoSensor->nro); 
				imprime(mensaje_imprime);
				snprintf(mensaje_log, LOG_SIZE, "Sensor \"%d\" ingresado", nuevoSensor->nro);
				registro(log, mensaje_log);
				break;
				
			case 12:	//buscamos un sensor en la lista
				printf("Ingrese el numero de sensor: ");
				scanf("\n%d", &numero);
				nuevoSensor = buscar_sensor(ptrCabeza, numero);	//devueve un puntero al sensor o NULL
				if(imprime_nodo(nuevoSensor) < 0)
				imprime("SENSOR NO ENCONTRADO");
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
	"2. Ingresar sensores por teclado.\n"	
	"3. Importar todos los sensores del archivo de configuracion \"%s\".\n"	
	"4. Importar un sensor desde el archivo de configuracion\n"				
	"5. Exportar sensores al archivo de configuracion \"%s\". \n"			
	"6. Borrar lista.\n"				
	"7. Borrar sensor.\n"			
	"8. Modificar sensor.\n"				
	"9. Simulador\n"					
	"10. Transmision de mediciones\n"	
	"11. Insertar un sensor\n"			
	"12. Buscar Sensor\n"				
	"13. Terminar ejecucion.\n"
	"Opcion ingresada: ", ARCHIVO_SENSORES, ARCHIVO_SENSORES);
	scanf("%d", &eleccion);
	return eleccion;
}//fin del menu



