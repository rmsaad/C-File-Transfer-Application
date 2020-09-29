/* A simple echo server using TCP */
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/signal.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <strings.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include <string.h>

#define SERVER_TCP_PORT 3000	/* well-known port */
#define BUFLEN		256	/* buffer length */

int echod(int);
void reaper(int);

int main(int argc, char **argv)
{
	
	int 	sd, new_sd, client_len, port;
	struct	sockaddr_in server, client;

	switch(argc){
	case 1:
		port = SERVER_TCP_PORT;
		break;
	case 2:
		port = atoi(argv[1]);
		break;
	default:
		fprintf(stderr, "Usage: %s [port]\n", argv[0]);
		exit(1);
	}

	/* Create a stream socket	*/	
	if ((sd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
		fprintf(stderr, "Can't creat a socket\n");
		exit(1);
	}

	/* Bind an address to the socket	*/
	bzero((char *)&server, sizeof(struct sockaddr_in));
	server.sin_family = AF_INET;
	server.sin_port = htons(port);
	server.sin_addr.s_addr = htonl(INADDR_ANY);
	if (bind(sd, (struct sockaddr *)&server, sizeof(server)) == -1){
		fprintf(stderr, "Can't bind name to socket\n");
		exit(1);
	}

	/* queue up to 5 connect requests  */
	listen(sd, 5);

	(void) signal(SIGCHLD, reaper);
	

	while(1) { 
	  client_len = sizeof(client);
	  new_sd = accept(sd, (struct sockaddr *)&client, &client_len);
	  if(new_sd < 0){
	    fprintf(stderr, "Can't accept client \n");
	    exit(1);
	  int i = 0;
	  i++;
 	  if (i == 1)
		fprintf(stdout, "got Here! \n");

	  }
	  switch (fork()){
	  case 0:		/* child */
		(void) close(sd);
		exit(echod(new_sd));
	  default:		/* parent */
		(void) close(new_sd);
		break;
	  case -1:
		fprintf(stderr, "fork: error\n");
	  }
	}
}

/*	echod program	*/
int echod(int sd)
{
	// STARTS HERE
	while(1){
	// variables
	char	*bp, buf[101] = {0};
	char buf2[100] = {0}, s[100] = {0};
	int 	n= 0, m = 0, o = 0, file_exists = 0, file_exists_flag = 0;
	
	// structures
	struct pdu{
		char type;
		int length;
		char data[100];
	}receive_pdu, send_pdu, write_pdu;
	
	// needed to scan directories
	struct dirent **namelist;
	
	// read data from socket and store it in receive_pdu
	n = read(sd, &receive_pdu, (int)sizeof(struct pdu));
	
	switch(receive_pdu.type){
		
		// file download case
		// check if file exists
		case 'D' : m = scandir(".", &namelist, NULL, alphasort);
					   while(m--){
					   file_exists = strcmp(receive_pdu.data, namelist[m]->d_name);
					   if (file_exists == 0)
						   file_exists_flag = 1;
					   }
					   
					  // check if files exists
					   if (file_exists_flag == 1){
						   // find length of file data
						   FILE* fp = fopen(receive_pdu.data, "r");
						   fseek(fp, 0L, SEEK_END);
						   send_pdu.length = ftell(fp);
						   fclose(fp);
						   
						   // read file data in pdu
						   int file = open(receive_pdu.data, O_CREAT | O_RDWR);
						   send_pdu.type = 'F';
						   o = read(file, send_pdu.data, 100); 
						   
						   // write pdu to socket
						   write(sd, &send_pdu, (int)sizeof(struct pdu));
							
							// close the file
						   close(file);
						   
					   // if file does not exist , send error pdu	   
					   }else{
						   send_pdu.type = 'E'; 
						   stpcpy(send_pdu.data, "File does not exist");
						   send_pdu.length =(int)strlen(send_pdu.data);
						    write(sd, &send_pdu, (int)sizeof(struct pdu));
					   }
					   break;
		
		// file upload case
		// check if file is already on server
		case 'U' : 	m = scandir(".", &namelist, NULL, alphasort);
						while(m--){
						file_exists = strcmp(receive_pdu.data, namelist[m]->d_name);
						if (file_exists == 0)
						   file_exists_flag = 1;
					   }
					   
						// if file is not on server, send ready pdu
						if (file_exists_flag == 0){
						   send_pdu.type = 'R';
						   strcpy(send_pdu.data, "0");
						   write(sd, &send_pdu, (int)sizeof(struct pdu));
							
							// read reponse from client
							read(sd, &write_pdu, (int)sizeof(struct pdu));
							
							// if pdu contains file data, write data to file
							if (write_pdu.type == 'F'){
								int file = open(receive_pdu.data, O_CREAT | O_RDWR, 0666);
								write(file, write_pdu.data, (write_pdu.length -1));
								close(file);
								printf("File Transfered\n");
							}
							
						// if file is already on server, send error pdu
						}else{
						   send_pdu.type = 'E'; 
						   stpcpy(send_pdu.data, "File is already on server");
						   send_pdu.length =(int)strlen(send_pdu.data);
						    write(sd, &send_pdu, (int)sizeof(struct pdu));
						}
						break;
						
		// change directory case
		// check if directory exists
		case 'P' :  m = scandir(".", &namelist, NULL, alphasort);
						while(m--){
						file_exists = strcmp(receive_pdu.data, namelist[m]->d_name);
						if (file_exists == 0)
						   file_exists_flag = 1;
						}
						// if directory exists change current directory
						if (file_exists_flag == 1){
							chdir(receive_pdu.data);
							printf("\nCurrent Directory:\n%s\n", getcwd(s, 100));
							send_pdu.type = 'R';
						    strcpy(send_pdu.data, "0");
							// send ready pdu
						    write(sd, &send_pdu, (int)sizeof(struct pdu));
						}
						break;
						
		// list files in a directory
		// scan for text files in specified directory
		case 'L' : 	m = scandir(receive_pdu.data, &namelist, NULL, alphasort);
						while (m--){
						char *find_txt = ".txt";
						char concat[100];
						// concatonate all text file names into pdu.data
						if (strstr(namelist[m]->d_name, find_txt) != NULL){
							strcat(send_pdu.data, namelist[m]->d_name);
							strcat(send_pdu.data, "\n");
						}
						} 
						// Debugging information
						printf("\nDebugging:\n%s\n", send_pdu.data);
						send_pdu.type = 'I';
						send_pdu.length =(int)strlen(send_pdu.data);
						
						// write to socket
						write(sd, &send_pdu, (int)sizeof(struct pdu));
						break;
						
		default : printf("Unexpected error\n"); break;
	}
	
	// initalize all arrays for reuse
	for(int i = 0; i < 100; i++){
			 buf[i] = 0; 
			 buf2[i] = 0;
			 s[i] = 0;
			 send_pdu.data[i] = 0;
			 receive_pdu.data[i] = 0; 
			 write_pdu.data[i]= 0;
		}
	}
	
	return(0);
}

/*	reaper		*/
void	reaper(int sig)
{
	int	status;
	while(wait3(&status, WNOHANG, (struct rusage *)0) >= 0);
}
