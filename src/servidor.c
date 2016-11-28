

/**
	\file servidor.c
	\brief El Servidor  recibe los datos desde los Clientes (RPI). Puede ser una PC, otra RPI o cualquier dispositivo adecuado que contenga el programa “servidor.c”.
	Este programa  realiza las siguientes operaciones:
	1. Crea un socket UDP y lo enlaza con un único puerto bien conocido por los Clientes y con cualquier dirección IP local válida.
	2. Espera la recepción de datos desde los Clientes.
	3. Al recibir datos utiliza la función put_to_send()para procesar el string recibido y adaptarla al siguiente formato:
	   {\"S_1_1\" : 0, \"S_1_2\" : 0, \"S_1_3\" : 0, \"S_2_1\" : 0, \"S_2_2\" : 0, \"S_2_3\" : 0, \"S_3_1\" : 0, \"S_3_2\" : 0, \"S_3_3\" : 0, \"S_3_4\" : 0, \"S_3_5\" : 0}
       éste formato se encuentra predeterminado en la declaración de la variable str_base, y es necesario para poder enviar los datos a la nube. Dicha función utiliza la subfunción loc_substr()que localiza el subtring dentro formato base y lo reemplaza por substring recibido desde el Cliente con los nuevos valores de medición. Finalmente llama a la subfunción reemplazar_substr()quien se encarga de hacer el reemplazo. Este proceso se puede realizar con múltiples Clientes al mismo tiempo.
	4. Toda la información recibida por el servidor en un lapso definido en TIME_SEND es enviado con el formato adecuado hacia un programa escrito en lenguaje Pyton mediante el uso de la librería Open Source <zeromq>(que permite conectar códigos en cualquier tipo de plataforma y llevar mensajes a través de TCP/IP). Éste programa retransmite los datos al servidor de “PubNub”. Finalmente, toda esta información es tomada desde el servidor de PubNub” en la nube por la página de la empresa “Freeboard” que nos permite armar un tablero de instrumentos (dashboard) donde se podrán ver las mediciones.
	
	COMPILACIÓN:     gcc -Wall -g servidor.c sensorlib.h -lzmq -o servidor
	  
	\author Darío Barone (barone.espindola.dario@gmail.com) - Maximiliano Bertotto (bertotto.maximiliano@gmail.com)
	\date 2016.12.26
	\version 1.0.0
*/





//LIBRERIAS-----------------------------------------------------------------------------------------------
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<ctype.h>
//#include<math.h>
#include<time.h>
#include<unistd.h>	
#include<netdb.h>
#include<sys/socket.h>
#include<arpa/inet.h>
#include<zmq.h>
#include<assert.h>



//#include"sensorlib.h"
#define SERVICE_PORT 21234		//puerto del servidor
#define SERVER_IP "127.0.0.2"	//IP del servidor
//#define TIMESLEEP 1
#define TIMEPRINT 400
#define BUF_SIZE 2048

void put_to_send(char str_base[], char str_recv[]);
void cargar_str(char* str_base, char *substr_recv);
char *reemplazar_substr(char *str, char *substr_orig, char *substr_remp);

int main(int argc, char **argv) {
	//variables
	struct sockaddr_in myaddr;	/* our address */
	struct sockaddr_in remaddr;	/* remote address */
	socklen_t addrlen = sizeof(remaddr);		/* length of addresses, pasamos &addrlen como argumento en recvfrom(); */
	int recvlen;			/* # bytes received, retorno de recvfrom(); */
	int fd;				/* our socket */
	char buf[BUF_SIZE];	/* receive buffer */
	time_t t_inicial;
	time_t t_final;
	char str_base[] =  "{\"S_1_1\" : 0, \"S_1_2\" : 0, \"S_1_3\" : 0, \"S_2_1\" : 0, \"S_2_2\" : 0, \"S_2_3\" : 0, \"S_3_1\" : 0, \"S_3_2\" : 0, \"S_3_3\" : 0, \"S_3_4\" : 0, \"S_3_5\" : 0}";                       
	//char str_transm[strlen(str_base)];
	char str_transm[1000];
	char str_recv[100] = {0};
	
	// a python (PubNub)
	void *context = zmq_ctx_new ();
    void *responder = zmq_socket (context, ZMQ_REP);
    int rc = zmq_bind (responder, "tcp://127.0.0.1:5555");
    assert (rc == 0);
	//char tmpbuf[250];
	//char buffer [250];
	//char tmpbuf[1000];
	char buffer [1000];

	

	//creamos socket UDP

	if ((fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
		perror("cannot create socket\n");
		return 0;
	}

	//enlazamos el socket con cuanquier direccion IP valida y con un puerto especifico
	memset((char *)&myaddr, 0, sizeof(myaddr));
	myaddr.sin_family = AF_INET;
	myaddr.sin_addr.s_addr = htonl(INADDR_ANY);	//cualquier IP
	myaddr.sin_port = htons(SERVICE_PORT);	//puerto conocido por los clientes

	if (bind(fd, (struct sockaddr *)&myaddr, sizeof(myaddr)) < 0) {
		perror("bind failed");
		return 0;
	}

	//bucle recepcion e impresion de datos
	t_inicial = clock();
	strcpy(str_transm, str_base);
	for (;;) {
		printf("waiting on port %d\n", SERVICE_PORT);
		recvlen = recvfrom(fd, buf, BUF_SIZE, 0, (struct sockaddr *)&remaddr, &addrlen);

		if (recvlen > 0) {
			buf[recvlen] = 0;
			//printf("%s (%d bytes)\n", buf, recvlen);
			str_recv[0] = '\0';
			strcpy(str_recv, buf);
			//printf("%s\n", str_recv);
			put_to_send(str_transm, str_recv);
			t_final = clock();		
			if ( (t_final - t_inicial) > TIMEPRINT ) {
				printf("envio a la nube\n");
				
				// Inicio envío a python
				
				//snprintf(tmpbuf, sizeof(tmpbuf), str_transm);
				//strcpy(tmpbuf, str_transm);
				zmq_recv (responder, buffer, 100, 0);
				printf ("Request Recieved\r\n");
				zmq_send (responder, str_transm, strlen(str_transm), 0);
				printf ("Responded with %s\r\n",str_transm);
				
				// Fin envío a python
				
				printf("%s\n", str_transm);
				str_transm[0] = '\0';
				strcpy(str_transm, str_base);
				t_inicial = clock();
				}//fin del if

		}
		else
			printf("uh oh - something went wrong!\n");

	}
	/* never exits */
}



/**
	\fn void put_to_send(char str_base[], char str_recv[])d)
	\brief Esta función divide el string recibido, lo divide en substrings y los va guardando en un formato predeterminado en función del número de rpi y del sensor.
	\author Darío Barone (barone.espindola.dario@gmail.com) - Maximiliano Bertotto (bertotto.maximiliano@gmail.com)
	\date 2016.12.26
	\return No retorna nada
	\bug Por el momento no encontrados
*/

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

/**
	\fn void cargar_str(char* str_base, char *substr_recv)
	\brief Carga string
	\author Darío Barone (barone.espindola.dario@gmail.com) - Maximiliano Bertotto (bertotto.maximiliano@gmail.com)
	\date 2016.12.26
	\return  No retorna nada
	\bug Por el momento no encontrados
*/

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

/**
	\fn char *reemplazar_substr(char *str, char *substr_orig, char *substr_remp)
	\brief Esta función reemplaza el substring original por el substring reemplazo.
	\author Darío Barone (barone.espindola.dario@gmail.com) - Maximiliano Bertotto (bertotto.maximiliano@gmail.com)
	\date 2016.12.26
	\return Retorna cero o uno
	\bug Por el momento no encontrados
*/

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

