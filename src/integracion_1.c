
/*
 * integracion_server_data_1.c
 * 
 *   Comp:  gcc -Wall -g integracion_1.c -lzmq -o integracion_1
 */

  

#include <zmq.h>
#include <stdio.h>
#include <time.h>
#include <unistd.h>
#include <assert.h>
#include <string.h>
 
int main (void)
{
    printf ("Sockets initializing\r\n");
    //  Socket to talk to clients
    void *context = zmq_ctx_new ();
    void *responder = zmq_socket (context, ZMQ_REP);
    int rc = zmq_bind (responder, "tcp://127.0.0.1:5555");
    assert (rc == 0);
 
	char tmpbuf[250];
	char buffer [250];

 
	while(1)
	{
        snprintf(tmpbuf, sizeof(tmpbuf), "{\"S_1_1\" : 59, \"S_1_2\" : 37, \"S_1_3\":5 , \"S_2_1\" : 23 , \"S_2_2\" : 73 , \"S_2_3\" : 3 , \"S_3_1\":17 , \"S_3_2\" : 5 , \"S_3_3\" : 93, \"S_3_4\" : 89 , \"S_3_5\" : 5}");
        zmq_recv (responder, buffer, 100, 0);
        printf ("Request Recieved\r\n");
        zmq_send (responder, tmpbuf, strlen(tmpbuf), 0);
        printf ("Responded with %s\r\n",tmpbuf);
 
	}
    return 0;
}
