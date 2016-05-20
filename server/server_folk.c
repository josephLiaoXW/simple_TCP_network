#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>
#include <netinet/in.h>

void doprocessing (int sock,char (*)[10],char (*)[50]);
int domain_to_ip(char *, char *);

int main( int argc, char *argv[] ) {
	int sockfd, newsockfd, portno, clilen;
	int reuseaddr = 1;
	int re_flag = 1;
	char buffer[256];
	struct sockaddr_in serv_addr, cli_addr;
	int n, pid;
	struct timeval timeout;
	timeout.tv_sec = 50;
	timeout.tv_usec = 0;
	/* First call to socket() function */
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0) {
		perror("ERROR opening socket");
		exit(1);
	}
   	if(setsockopt(sockfd,SOL_SOCKET, SO_REUSEADDR,&re_flag,sizeof(re_flag))==-1){
		printf("reuse failed!!");
		exit(1);
	}
	if(setsockopt(sockfd,SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout,sizeof(timeout))<0){
		perror("set timeout failed\n");
		exit(1);
	}
	/* Initialize socket structure */
	bzero((char *) &serv_addr, sizeof(serv_addr));
	portno = 5005; 
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = INADDR_ANY;
	serv_addr.sin_port = htons(portno);
	/* Now bind the host address using bind() call.*/
	if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
		perror("ERROR on binding");
		exit(1);
	}
	/*Preprocess the database file*/
	FILE *fp;
	fp = fopen("query.txt","r");
	if(!fp){
		printf("database open ERR!\n");
		exit(1);
	}
	char sentence[200];
	char stu_id[3][10];
	char stu_mail[3][50];
	int stu_num = 0;
	while(!feof(fp)){
		fscanf(fp,"%s%s",stu_id[stu_num],stu_mail[stu_num]);
		stu_num++;
	}
	fclose(fp);
	/* Now start listening for the clients, here
      * process will go in sleep mode and will wait
      * for the incoming connection
   */
   
   listen(sockfd,5);
   clilen = sizeof(cli_addr);
   
   while (1) {
      newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
		
      if (newsockfd < 0) {
         perror("ERROR on accept");
         exit(1);
      }
      
      /* Create child process */
      pid = fork();
		
      if (pid < 0) {
         perror("ERROR on fork");
         exit(1);
      }
      
      if (pid == 0) {
         /* This is the client process */
         close(sockfd);
         doprocessing(newsockfd,stu_id,stu_mail);
         exit(0);
      }
      else {
         close(newsockfd);
      }
		
   } /* end of while */
}
void doprocessing (int sock,char stu_id[][10],char stu_mail[][50]) {
   int n;
   char buffer[256];
   while(1){
   		bzero(buffer,256);
   		n = read(sock,buffer,255);
   		if (n < 0) {
			perror("ERROR reading from socket");
			exit(1);
		}
		char req[] = "a_list";
		int list_req = strcmp(buffer,req);
		if(list_req==0){
		/*reply the service list*/
		n = write(sock,"What's your requirement? 1.DNS 2.QUERY 3.QUIT :",48);
   	 	if (n<0){
				perror("ERROR writting to socket");
				exit(1);
			}
			/*Reply the service depend on service num*/
			bzero(buffer,256);
			n = read(sock,buffer,255);
			if (n<0) {
				perror("ERROR reading from socket");
				exit(1);
			}
			char *service_str[3];
			service_str[0] = "1";
			service_str[1] = "2";
			service_str[2] = "3";
			int proid = 0,service_num=-1;
			for(proid=0;proid<3;proid++){
				if(strcmp(service_str[proid],buffer) == 0){
					service_num = proid+1;
				}
			}
			switch(service_num){
				case 1:
					n = write(sock,"Input URL address :",20);
					break;
				case 2:
					n = write(sock,"Input Student ID :",18);
					break;
				case 3:
					break;
				default:
					n = write(sock,"Error request num!!",20);
					break;
			}
			if(n<0){
				perror("ERROR writting to address");
				exit(1);
			}
			/*Depend on servive number, resposnse different result*/
			if(service_num==1){
				bzero(buffer,256);
				n = read(sock,buffer,255);
				if (n<0) {
					perror("ERROR reading from socket");
					exit(1);
				}
				char ip[20]="";
				domain_to_ip(buffer, ip);
				/*reply DNS result*/
				n=write(sock,ip,strlen(ip));
				if(n<0){
					perror("ERROR writting to address");
					exit(1);
				}
			}
			if(service_num==2){
				bzero(buffer,256);
				n = read(sock,buffer,255);
				if(n<0){
					perror("ERROR reading from socket");
					exit(1);
				}
				int stu_num;
				for(stu_num=0;stu_num<3;stu_num++){
					if(!strcmp(stu_id[stu_num],buffer)){
						n=write(sock,stu_mail[stu_num],strlen(stu_mail[stu_num]));
						if(n<0){
							perror("ERROR writting to address");
							exit(1);
						}
					stu_num=4;
					}
				}
				if(stu_num==3){
					n=write(sock,"No such student ID",19);
					if(n<0){
						perror("ERROR writting to address");
						exit(1);
					}
				}
			}
		}	
	}		/*end of while*/
}
int domain_to_ip(char *domain,char *ip){
	int i=0;
	struct hostent *dm;
	struct in_addr **addr;
	if ((dm = gethostbyname(domain)) == NULL){
		printf("Failed to resolve!!\n");
		herror("gethostbyname");
		strcpy(ip,"failed\0");
		return 1;
	}
	addr =(struct in_addr **)dm->h_addr_list;
	for( i =0;addr[i] != NULL;i++){
		strcpy(ip , inet_ntoa(*addr[i]));
		return 0;
	}
}
