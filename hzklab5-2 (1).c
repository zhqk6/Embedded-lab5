/*
 * hzklab5-2.c
 *
 *  Created on: Apr 5, 2016
 *      Author: zhqk6
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>

void error(const char *msg){
    perror(msg);
    exit(0);
}

int main(int argc, char*argv[]){
	struct sockaddr_in serv_addr;
	struct sockaddr_in anybodyaddr;
	int port;
	int boolval=1;
	socklen_t fromlen;
	if (argc < 2){
		  printf("usage: %s port\n", argv[0]);
	      exit(0);
	}
	// argv[0] is our source file ID, argv[1] is our port number
	int sock=socket(AF_INET,SOCK_DGRAM,0);
	if(sock<0){
		error("ERROR opening socket");
	}
    // create our socket
	char hostname[40];
	char myaddr[10];
	struct hostent *host;
	struct in_addr **boardIPlist;
	gethostname(hostname,sizeof(hostname));
	host = (struct hostent*)gethostbyname(hostname);
	boardIPlist=(struct in_addr**)(host->h_addr_list);
	strcpy(myaddr,inet_ntoa(*boardIPlist[0]));
    // get our local IP address

	port=atoi(argv[1]);
	bzero(&serv_addr,sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = INADDR_ANY;
	serv_addr.sin_port = htons(port);
	if(bind(sock,(struct sockaddr*)&serv_addr,sizeof(serv_addr))<0){
		error("binding");
	}
	// bind our socket

	if(setsockopt(sock,SOL_SOCKET,SO_BROADCAST,&boolval,sizeof(boolval))<0){
		printf("error setting socket options\n");
		exit(-1);
	}
	// set socket so that it can broadcast messages

	fromlen=sizeof(struct sockaddr_in);
	int random;

	//obtain random number which means the votes we get
	char s[2];
	char addrrandom[15];
	char buffer[40];
	char buff1[]="WHOIS";
	char buff2[]="VOTE";
	//the message we need to check below
	int R=2;
	int K=0;
	//the lock of whether we are the master
	//if R=0, we are the slave, else we are the master
	while(1){
		bzero(buffer,40);
		recvfrom(sock,buffer,40,0,(struct sockaddr*)&anybodyaddr,&fromlen);
		//receive messages from client. it will be blocked if no messages are received
        if((strstr(buffer,buff1)!=NULL)&&(R==1)){
        	//not only when we receive WHOIS but also we are the master can we send

        	anybodyaddr.sin_addr.s_addr=inet_addr("10.3.52.255");
        	bzero(buffer,40);
        	strcpy(buffer,"Huang on board ");
        	strcat(buffer,myaddr);
        	strcat(buffer," is the master");
        	//Huang on board 10.3.52.X is the master if I am the master
            sendto(sock,buffer,40,0,(struct sockaddr*)&anybodyaddr,fromlen);
            //send the message above to client
        }
        else if(strstr(buffer,buff2)!=NULL){
        	anybodyaddr.sin_addr.s_addr=inet_addr("10.3.52.255");
        	srand(time(NULL));
        	random=1+rand()%10;
        	sprintf(s,"%d",random);
        	strcpy(addrrandom,"# ");
        	strcat(addrrandom,myaddr);
        	strcat(addrrandom," ");
        	strcat(addrrandom,s);
        	//combine my local ip address to the random vote number
        	//the result string will be like # 10.3.52.X Y
        	sendto(sock,addrrandom,15,0,(struct sockaddr*)&anybodyaddr,fromlen);
        	// send my local ip and random vote numbers to client
        	bzero(buffer,40);
        	K=1;// to indicate that voting has been executed
        	R=1;// if no IP, I am the master
        }
        else if(buffer[0]=='#'){
             if(atoi(&buffer[12])>atoi(&addrrandom[12])){
            	 R=0;
            	 K=0;
            	 //if I am a slave after comparing, I will never be the master unless we vote again
             }
             else if(atoi(&buffer[12])<atoi(&addrrandom[12])){
            	 if((R==0)&&(K==0)){
            	 R=0;
            	 }
            	 else{
                 R=1;
            	 }
             }
             //compare vote numbers
             else if(atoi(&buffer[12])==atoi(&addrrandom[12])){
            	 // when tie occurs, compare last bits of ip address
                 if(atoi(&buffer[10])>atoi(&addrrandom[10])){
                	 R=0;
                	 K=0;
                	 //if I am a slave after comparing, I will never be the master unless we vote again
                 }
                 else if(atoi(&buffer[10])<atoi(&addrrandom[10])){
                	 if((R==0)&&(K==0)){
                	    R=0;
                	   }
                	    else{
                	    R=1;
                	}
                 }
             }
        }
        fflush(stdout);
	}
	return 0;
}
