//compile with: gcc server.c sleep_time.c -o server  -lpthread
//run with: ./server
//note: if connection is lost, restart both the ./server and ./client


#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <pthread.h>
#include "sleep_time.h"

#define MY_PORT	8765

void *fast_recieve(void *ptr); // fast loop... intended to receive the force data
void *slow_send(void *ptr); // slow loop... intended for sending position measurements

// needed for socket creation and usage
int serverFd, connectionFd;
struct sockaddr_in servaddr;
char in_buffer[1024], out_buffer[1024];
double var[3] = {1.1,2.2,3.3}, recv_data[10] = {0.0};
int _true = 1;

int main(void)
{
	int rc1, rc2; // For the returned values
	pthread_t thread1, thread2; // Threads pointer's
	
	// create socket
	if ( (serverFd = socket( PF_INET, SOCK_STREAM, 0 )) == -1 ) {
		printf("Error creating socket\n");
		exit(1);
	}
	
	// set socket port options
	if ( setsockopt(serverFd, SOL_SOCKET, SO_REUSEADDR, &_true, sizeof(int)) == -1 ) {
		printf("Error setting socket port options\n");		
		exit(1);
	}
	
	// initialise address' structure
	memset( &servaddr, 0, sizeof(servaddr) ); // reset memory to 0's
	servaddr.sin_family = AF_INET; // IPv4
	servaddr.sin_addr.s_addr = htonl( INADDR_ANY ); // host to network long ----------------------------------------
	servaddr.sin_port = htons( MY_PORT ); // port

	// bind address with socket
	if ( bind( serverFd, (struct sockaddr *)&servaddr, sizeof(servaddr) ) == -1 ) {
		printf("Error binding socket\n");
		exit(1);
	}
	
	// listen to max 5 sockets
	if ( listen( serverFd, 5 ) == -1 ) {
		printf("Error setting listen function\n");
		exit(1);
	}
	
	// new connected socket... WAITS HERE UNTIL CONNECTED
	printf("Waiting for connection\n");
	if ( (connectionFd = accept( serverFd, (struct sockaddr*)NULL, NULL )) <= 0 ) {
		printf("Error accepting connection\n");
		exit(1);
	}
	else
		printf("Connection established\n");
	
	// create fast thread 
	if( (rc1=pthread_create( &thread1, NULL, &fast_recieve, NULL )) )
	{
		printf( "Thread creation failed: %d\n", rc1 );
		exit(1);
	}

	// create slow thread
	if( (rc2=pthread_create( &thread2, NULL, &slow_send, NULL )) )
	{
		printf( "Thread creation failed: %d\n", rc2 );
		exit(1);
	}

	/* Wait till threads are complete before main continues. Unless we  */
	/* wait we run the risk of executing an exit which will terminate   */
	/* the process and all threads before the threads have completed.   */
	pthread_join( thread1, NULL ); 
	pthread_join( thread2, NULL ); 

	close( connectionFd	); // close connection with client 
	close( serverFd ); // close server socket
	printf("\n closed connection\n");

	return 0;
}

//~ // To receive force data
void *fast_recieve(void *ptr)
{
	while (1) {
		//~ memset( &in_buffer, 0, sizeof(in_buffer) ); // reset memory to 0's
		//~ recv( connectionFd, in_buffer, sizeof(in_buffer), 0 ); // receive message
		//~ sscanf( in_buffer, "%lf %lf %lf %lf %lf %lf %lf %lf %lf %lf ", &recv_data[0], &recv_data[1], &recv_data[2], &recv_data[3], &recv_data[4], &recv_data[5], &recv_data[6], &recv_data[7], &recv_data[8], &recv_data[9] );
		//~ printf( "Received: %4.2lf %4.2lf %4.2lf %4.2lf %4.2lf %4.2lf %4.2lf %4.2lf %4.2lf %4.2lf\n", recv_data[0], recv_data[1], recv_data[2], recv_data[3], recv_data[4], recv_data[5], recv_data[6], recv_data[7], recv_data[8], recv_data[9] );		
	 }
}

void *slow_send(void *ptr)
{
	while (1) {
		memset( &out_buffer, 0, sizeof(out_buffer) ); // reset memory to 0's
		//~ sprintf( out_buffer, "%f %f\n", var[0], var[1] ); // fill buffer   ----var are data to send
		sprintf( out_buffer, "%c %c\n", 'i', 'o' ); // fill buffer   ----var are data to send
		send( connectionFd, out_buffer, strlen(out_buffer), 0 ); // send the message
		sleep_time(0.1); // wait 1 sec
	}
}
