#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <errno.h>
#include <netdb.h>
#define MESLEN 256
#define NICKLEN 8
#define MAX_C 10

typedef struct {
	    int sock_fd;
	    char nick[NICKLEN];			
        char message[MESLEN];
        char privates[MAX_C*NICKLEN];
    }client_struct;

    client_struct R_USER, S_USER;
    int port, sockfd, i;

void hndlr(int s){ //CTRL+C
        memset(R_USER.message,'\0',sizeof(R_USER.message));
        memset(R_USER.nick,'\0',sizeof(R_USER.nick));
        strcpy(R_USER.nick,"server");
        strcpy(R_USER.message,"*** User ");
        strcat(R_USER.message,S_USER.nick);
        strcat(R_USER.message," left us without any message\n");
        send(sockfd, &R_USER, sizeof(R_USER), 0);
        shutdown(sockfd,2);
        close(sockfd);
        _exit(0);
    }
int main(int argc, char** argv){
	struct			hostent *host;
	struct sockaddr_in 	ser_adr;

    signal(SIGINT,hndlr);
    signal(SIGTSTP,hndlr);
    //input parameters
	if (argc != 4) {
		fprintf(stderr, "Parameter error:\n");
		exit(1);
	}

	if (!(host = gethostbyname(argv[1]))) {
		fprintf(stderr, "Host error: %s\n", argv[1]);
		exit(1);
	}

	if (sscanf(argv[2], "%d", &port) != 1 || port <= 1023) {
		fprintf(stderr, "Port error: %s\n", argv[2]);
		exit(1);
	}

    if(strcpy(S_USER.nick,argv[3])<0){
        fprintf(stderr, "nick error: %s\n", argv[3]);
		exit(1);
    }
    //socket prepairing
	if ((sockfd = socket(PF_INET, SOCK_STREAM, 0)) < 0) {
		perror("socket");
		exit(1);
	}

	ser_adr.sin_family = AF_INET;
	ser_adr.sin_port = htons(port);
	memcpy(&ser_adr.sin_addr, host->h_addr_list[0], sizeof(ser_adr.sin_addr));
	if (connect(sockfd, (struct sockaddr*) &ser_adr, sizeof(ser_adr)) < 0) {
		perror("connect");
		exit(1);
	}
    send(sockfd,&S_USER,sizeof(S_USER),MSG_DONTWAIT);
    fd_set readfds, all;
    int max_d=sockfd, j, k;
    char c;
    FD_ZERO(&all);
    FD_ZERO(&readfds);
    FD_SET(0, &all);
    FD_SET(sockfd, &all);

    while(1){
		readfds = all;
		if(select(max_d+1, &readfds, NULL, NULL, NULL) == -1){
			perror("select");
			exit(-1);
		}
                if(FD_ISSET(0, &readfds)){
                    if(fgets(S_USER.message, MESLEN, stdin)==NULL){
                        perror("input message");
                        exit(-1);
                    }
                    for(j=0;j<MESLEN;j++){
                        if(S_USER.message[j]=='\n') break;
                    }
                    if(j!=MESLEN){
                        for(j=0;S_USER.message[j]==' ';j++){            //spaces:begin
                            S_USER.message[j]='\0';
                        }
                        if(j){
                            for(k=0;k<MESLEN-j;k++)
                                S_USER.message[k]=S_USER.message[k+j];
                                S_USER.message[k+j]='\0';
                        }
                        for(j=MESLEN-1;S_USER.message[j]!='\n';j--);
                        while(S_USER.message[j-1]==' '){
                            S_USER.message[j-1]='\n';
                            S_USER.message[j]='\0';
                            j--;
                        };                                              //spaces:end
                        if(strcmp(S_USER.message,"\n")){
                            if(S_USER.message[0]=='\\'){  //deleting excess spaces in cmd
                                for(j=0;j<MESLEN;j++)
                                    if(S_USER.message[j]==' ' && S_USER.message[j+1]==' '){
                                        for(k=j+1;k<MESLEN-1;k++)
                                            S_USER.message[k]=S_USER.message[k+1];
                                        S_USER.message[MESLEN-1]='\0';
                                        j--;
                                    }
                            }

                            send(sockfd, &S_USER, sizeof(S_USER), 0);
                            if (S_USER.message[0]=='\\' && S_USER.message[1]=='q' && S_USER.message[2]=='u' && S_USER.message[3]=='i' && S_USER.message[4]=='t' && S_USER.message[5]==' ') {
                                shutdown(sockfd,2);
                                close(sockfd);
			                    exit(0);
		                    }
                        } else printf("Your message is empty:(\n");
                    } else {
                        printf("Your message is too long!\n");
                        while((c=getchar())!='\n');
                        }
                } 
                if(FD_ISSET(sockfd, &readfds)){
                    if(recv(sockfd, &R_USER, sizeof(R_USER), 0)>0){
                        if(strcmp(R_USER.message,"\0")){
                            printf("%s:%s", R_USER.nick, R_USER.message);
		                    fflush(stdout);
                        }
                    } else{
                        shutdown(sockfd,2);
                        close(sockfd);
                        return 0;
                    }
                }
            
	}
	close(sockfd);
    return 0;
}