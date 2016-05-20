#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <netdb.h>
#include <netinet/in.h>

#include <string.h>

int main(int argc, char *argv[]) {
   int sockfd, portno, n;
   struct sockaddr_in serv_addr;
   struct hostent *server;
   
   char buffer[256];
   
   if (argc < 3) {
      fprintf(stderr,"usage %s hostname port\n", argv[0]);
      exit(0);
   }
	
   portno = atoi(argv[2]);  
   
   /* Create a socket point */
   sockfd = socket(AF_INET, SOCK_STREAM, 0);
   
   if (sockfd < 0) {
      perror("ERROR opening socket");
      exit(1);
   }
	
   server = gethostbyname(argv[1]);
   
   if (server == NULL) {
      fprintf(stderr,"ERROR, no such host\n");
      exit(0);
   }
   
   bzero((char *) &serv_addr, sizeof(serv_addr));
   serv_addr.sin_family = AF_INET;
   bcopy((char *)server->h_addr, (char *)&serv_addr.sin_addr.s_addr, server->h_length);
   serv_addr.sin_port = htons(portno);
   
   /* Now connect to the server */
   if (connect(sockfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
      perror("ERROR connecting");
      exit(1);
   }
    /*start loop*/
	int quit=0;
	char request[7] = "a_list";
	request[7] = '\0';
	while(!quit){
	/* sent a request for service list */   
		n = write(sockfd, request, strlen(request));
   
	    if (n < 0) {
    		perror("ERROR writing to socket");
			exit(1);
    	}
    	/* Now read server response */
    	bzero(buffer,256);
    	n = read(sockfd, buffer, 255);
		if (n < 0) {
			perror("ERROR reading from socket");
			exit(1);
		}
	
		printf("%s",buffer);
		/* Reply the service number */
		char req_num[4]="";
		scanf("%s",req_num);
		int service = atoi(req_num);
		/* Send message to the server */
   		n = write(sockfd, req_num, strlen(req_num));
   
   		if (n < 0) {
      		perror("ERROR writing to socket");
      		exit(1);
   		}
   
   		/* Now read server response */
		if(service==1 || service==2){
			bzero(buffer,256);
			n = read(sockfd, buffer, 255);
   
			if (n < 0) {
				perror("ERROR reading from socket");
				exit(1);
			}
			printf("%s",buffer);
		}
		if(service==1){ //DNS service
   			char domain[50];
			scanf("%s",domain);
			n = write(sockfd, domain, strlen(domain));
			if(n < 0){
				perror("ERROR writting to socket");
				exit(1);
			}
			bzero(buffer,256);
			n = read(sockfd,buffer,255);
			if(n < 0){
				perror("ERROR writting to socket");
				exit;
			}
			if(strcmp(buffer,"failed")!=0)
				printf("address get from domain name :%s\n",buffer);
		}
		else if(service==2){
			char stu_id[10];
			scanf("%s",stu_id);
			n = write(sockfd,stu_id,strlen(stu_id));
			if(n<0){
				perror("ERROR writting to socket");
				exit(1);
			}
			bzero(buffer,256);
			n = read(sockfd,buffer,255);
			if(n<0){
				perror("ERROR reading from socket");
				exit;
			}
			printf("Email get from server : %s\n",buffer);
		}
		else if(service==3){
			quit=1;
			return 0;
		}
		getchar();
		getchar();
		
	}
	return 0;
}
