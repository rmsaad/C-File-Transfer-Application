/* A simple echo client using TCP */
#include <stdio.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <strings.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <string.h>



#define SERVER_TCP_PORT 3000	/* well-known port */
#define BUFLEN		256	/* buffer length */

int main(int argc, char **argv)
{
	int 	n, i, bytes_to_read;
	int 	sd, port;
	struct	hostent		*hp;
	struct	sockaddr_in server;
	char	*host, *bp, rbuf[BUFLEN], sbuf[BUFLEN];

	switch(argc){
	case 2:
		host = argv[1];
		port = SERVER_TCP_PORT;
		break;
	case 3:
		host = argv[1];
		port = atoi(argv[2]);
		break;
	default:
		fprintf(stderr, "Usage: %s host [port]\n", argv[0]);
		exit(1);
	}

	/* Create a stream socket	*/	
	if ((sd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
		fprintf(stderr, "Can't creat a socket\n");
		exit(1);
	}

	bzero((char *)&server, sizeof(struct sockaddr_in));
	server.sin_family = AF_INET;
	server.sin_port = htons(port);
	if (hp = gethostbyname(host)) 
	  bcopy(hp->h_addr, (char *)&server.sin_addr, hp->h_length);
	else if ( inet_aton(host, (struct in_addr *) &server.sin_addr) ){
	  fprintf(stderr, "Can't get server's address\n");
	  exit(1);
	}

	/* Connecting to the server */
	if (connect(sd, (struct sockaddr *)&server, sizeof(server)) == -1){
	  fprintf(stderr, "Can't connect \n");
	  exit(1);
	}
	
	// STARTS HERE
	while(1){
	
		// variables
		char temp_buf[100]  = {0};
		char temp_buf2[100] = {0};
		char s[100];
		int n = 0, m = 0, o = 0, choice = 0;
		char c = 0;
		
		// PDUs
		struct pdu{
		char type;
		int length;
		char data[100];
		}request_pdu = {0}, receive_pdu = {0}, send_pdu = {0};
		
		
		// user decides on action
		printf("\nWhat would you like to Do?\n");
		printf("(1) File Download\n(2) File Upload\n(3) Change Directory\n(4) List Directory\n(5) Exit Program\nans:"); 
		
		// ensure they choose a valid option
		while(scanf("%d%c", &choice, &c) && choice != 0 && choice > 5){
			printf("retry: ");		
		}
		
		// switch case
		switch(choice){
			// file download case
			case 1 : printf("Enter the correct file name to Download\n"); 
						
						// read stdin data into a temporary buffer
						n = read(0, temp_buf, 100); temp_buf[n-1] = '\0';
						
						// copy data into pdu, assign a type and length
						stpcpy(request_pdu.data, temp_buf);
						request_pdu.type = 'D';
						request_pdu.length =(int)strlen(temp_buf);
						
						// Debugging information
						 printf("\nPDU sent to server:\ntype:%c\nlength:%d\ndata:%s\n", request_pdu.type, request_pdu.length, request_pdu.data);
						
						// write pdu to socket
						write(sd, &request_pdu, (int)sizeof(struct pdu));
						
						// read socket and store in receive_pdu
						m = read (sd, &receive_pdu, (int)sizeof(struct pdu));
						
						// Debugging information
						 printf("\nPDU received from server:\ntype:%c\nlength:%d\ndata:%s\n", receive_pdu.type, receive_pdu.length, receive_pdu.data);
						
						// if pdu contains file data, write data to file
						if (receive_pdu.type == 'F'){
								int file = open(request_pdu.data, O_CREAT | O_RDWR, 0666);
								write(file, receive_pdu.data, (receive_pdu.length -1));
								close(file);
								printf("\nFile Transfered\n");
						}
						// file does not exist, print out error message
						if(receive_pdu.type == 'E'){
							printf("\nError Message:\n%s\n", receive_pdu.data);
						}
						
						break;
			
			// file upload case
			case 2 : printf("Enter the correct file name to Upload\n"); 
						 
						 // read stdin data into a temporary buffer
						n = read(0, temp_buf2, 100); temp_buf2[n-1] = '\0';
						
						// copy data into pdu, assign a type and length
						stpcpy(send_pdu.data, temp_buf2);
						send_pdu.type = 'U';
						send_pdu.length =(int)strlen(temp_buf2);
						
						// write pdu to socket
						write(sd, &send_pdu, (int)sizeof(struct pdu));
						
						// Debugging information
						 printf("\nPDU sent to server:\ntype:%c\nlength:%d\ndata:%s\n", send_pdu.type, send_pdu.length, send_pdu.data);
						
						// read response in pdu
						m = read (sd, &receive_pdu, (int)sizeof(struct pdu));
						
						// Debugging information
						 printf("\nPDU received from server:\ntype:%c\ndata:%s\n", receive_pdu.type, receive_pdu.data);
						
						// check if ready pdu is received
						if (receive_pdu.type == 'R'){
						  // find file length
						   FILE* fp = fopen(send_pdu.data, "r");
						   fseek(fp, 0L, SEEK_END);
						   request_pdu.length = ftell(fp);
						   fclose(fp);
							
						   // read file data into pdu, assign type
						   int file = open(send_pdu.data, O_CREAT | O_RDWR);
						   request_pdu.type = 'F';
						   o = read(file, request_pdu.data, 100); 
						   // write pdu to socket
						   write(sd, &request_pdu, (int)sizeof(struct pdu));
							
						   // Debugging information
						   printf("\nPDU sent to server:\ntype:%c\nlength:%d\ndata:%s\n", request_pdu.type, request_pdu.length, request_pdu.data);
						
						   // close the file
						   close(file);
						}
						// if error pdu is recieved, print message
						if(receive_pdu.type == 'E'){
							printf("\nError message:\n%s\n", receive_pdu.data);
						}
						
						 break;
			
			// change directory case
			case 3 : printf("Enter the correct Directory Name\n"); 
						 
						// read stdin data into a temporary buffer
						n = read(0, temp_buf, 100); temp_buf[n-1] = '\0';
						
						// copy data into pdu, assign a type and length
						stpcpy(request_pdu.data, temp_buf);
						request_pdu.type = 'P';
						request_pdu.length =(int)strlen(temp_buf);
						
						// write pdu to socket
						write(sd, &request_pdu, (int)sizeof(struct pdu));
						
						// receive ready pdu
						m = read (sd, &receive_pdu, (int)sizeof(struct pdu));
						
						// Debugging information
						 printf("\nPDU received from server:\ntype:%c\ndata:%s\n", receive_pdu.type, receive_pdu.data);
						
						 break;
			
			// list files in a directory
			case 4 : printf("Specify Directory\n"); 
						 
						 // read stdin data into a temporary buffer
						n = read(0, temp_buf2, 100); temp_buf2[n-1] = '\0';
						
						// copy data into pdu, assign a type and length
						stpcpy(request_pdu.data, temp_buf2);
						request_pdu.type = 'L';
						request_pdu.length =(int)strlen(temp_buf2);
						
						// Debugging information
						printf("\nPDU sent to server:\ntype:%c\nlength:%d\ndata:%s\n", request_pdu.type, request_pdu.length, request_pdu.data);
						
						// write pdu to socket
						write(sd, &request_pdu, (int)sizeof(struct pdu));
						
						m = read (sd, &receive_pdu, (int)sizeof(struct pdu));
						// print out all files in directory
						printf("\nFiles in Directory\n%s\n", receive_pdu.data);
						break;
			
			
			case 5 : printf("Goodbye\n"); 
						 
						// exit program
						return(0); 
						break;
			
			default:printf("Critical Error\n"); 
						
						break;
		}
		// initalize all arrays for reuse
		for(int i = 0; i < 100; i++){
			 temp_buf[i] = 0; 
			 temp_buf2[i] = 0;
			 request_pdu.data[i] = 0;
			 receive_pdu.data[i] = 0; 
			 send_pdu.data[i]= 0;
		}
	}
}
