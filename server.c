#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <signal.h>
#define MAX_C 10
#define BUFSIZE 256
#define MESLEN 256
#define NICKLEN 8

    typedef struct {
	    int sock_fd;
	    char nick[NICKLEN];
        char message[MESLEN];
        char privates[MAX_C*NICKLEN];
    }client_struct;

    client_struct clients[MAX_C], RECIEVE, NEW, CMD;
    int port, sockfd, newsockfd, opt = 1,i, j, k, CUR_C=0, recvd;

    void hndlr(int s){ //CTRL+C
        int j;
        memset(RECIEVE.message,'\0',sizeof(RECIEVE.message));
        memset(RECIEVE.nick,'\0',sizeof(RECIEVE.nick));
        strcpy(RECIEVE.message,"### server is shutting down, thanks to everyone\n");
        strcpy(RECIEVE.nick,"server");
        for(j=0;j<MAX_C;j++)
            if(clients[j].sock_fd>0 && clients[j].sock_fd !=sockfd)
                if(send(clients[j].sock_fd,&RECIEVE,sizeof(RECIEVE),0)==-1)
                    perror("send");
    
        for(j=0;j<MAX_C;j++)
            if(clients[j].sock_fd>0){
                shutdown(clients[i].sock_fd,2);
                close(clients[i].sock_fd);
            }
        _exit(0);
    }

int main(int argc, char** argv){
    
    struct sockaddr_in ser_adr, cli_adr;
    socklen_t cli_adr_len = sizeof(cli_adr);



    for( j=0;j<MAX_C;j++){
        clients[j].sock_fd=0;
        memset(clients[j].message,'\0',sizeof(RECIEVE.message));
        memset(clients[j].nick,'\0',sizeof(RECIEVE.nick));
        memset(clients[j].privates,'\0',sizeof(RECIEVE.privates));
        
    }
    signal(SIGINT,hndlr);
    signal(SIGTSTP,hndlr);
    //input parameters
    if(argc!=2){
        fprintf(stderr, "Parameter error");
        exit(1);
    }
    if (sscanf(argv[1], "%d", &port) != 1 || port <= 1023) {
		fprintf(stderr, "Port error: %s\n", argv[1]);
		exit(1);
	}
    //socket prepairing
    if ((sockfd = socket(PF_INET, SOCK_STREAM, 0)) < 0) {
		perror("sock_creat");
		exit(1);
	}
	if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
		perror("setsockopt");
		exit(1);
	}
    ser_adr.sin_family = AF_INET;
	ser_adr.sin_port = htons(port);
	ser_adr.sin_addr.s_addr = INADDR_ANY;
	if (bind(sockfd, (struct sockaddr *) &ser_adr, sizeof(ser_adr)) < 0) {
		perror("bind");
		exit(1);
	}
	if (listen(sockfd, 5) < 0) {
		perror("listen");
		exit(1);
	}
    printf("\nServer is working now\n");
	fflush(stdout);
    //
    //
    //
    //
    
    for(; ;){
        fd_set readfds;
        int max_d=sockfd, flag;
        FD_ZERO(&readfds);
        FD_SET(sockfd,&readfds);
        printf("-----------iteration-----------\n");
        printf("adding\n");
        for(i=0;i<MAX_C;i++){ //adding ACTIVE users to readfds

            if(clients[i].sock_fd>0){
                printf("%d added\n",i);
                FD_SET(clients[i].sock_fd, &readfds);
                if(clients[i].sock_fd > max_d)
                    max_d=clients[i].sock_fd;
            }

        }
        printf("selecting\n");
        if(select(max_d+1, &readfds, NULL, NULL, NULL) == -1){ //checking
			perror("select");
			exit(1);
		}
        printf("new connectin?\n");
        if(FD_ISSET(sockfd, &readfds)){// new connection?!

            if((newsockfd = accept(sockfd, (struct sockaddr *) &cli_adr, &cli_adr_len)) == -1) {
		        perror("accept");
		        exit(1);
	        }

            memset(NEW.message,'\0',sizeof(NEW.message));
            memset(NEW.nick,'\0',sizeof(NEW.nick));

            if(CUR_C==MAX_C){ //maximum reached:(
                printf("### max clients reached\n");
                printf("### connection from %s on port %d refused\n",inet_ntoa(cli_adr.sin_addr), ntohs(cli_adr.sin_port));
                strcpy(NEW.nick,"server");
                strcpy(NEW.message,"*** max clients reached");
                send(newsockfd,&NEW,sizeof(NEW),MSG_DONTWAIT);
                shutdown(newsockfd,2);
                close(newsockfd);
                continue;
            } else{ //new user:)
                recv(newsockfd,&NEW,sizeof(NEW),MSG_DONTWAIT);
                i=0;
                while(i<MAX_C && strcmp(clients[i].nick,NEW.nick)){i++;}
                if(i==MAX_C){ //nick is free - new user
                    CUR_C++;
                    i=0;
                    while(clients[i].sock_fd>0){i++;}
                    //clients[i].addr=cli_adr;
                    clients[i].sock_fd=newsockfd;
                    strcpy(clients[i].nick,NEW.nick);

                    memset(NEW.message,'\0',sizeof(NEW.message));
                    memset(NEW.nick,'\0',sizeof(NEW.nick));

                    strcpy(NEW.nick,"server");
                    strcpy(NEW.message,"*** New user on server. Hello user: ");
                    strcat(NEW.message,clients[i].nick);
                    strcat(NEW.message,"\n");
                    for(j=0;j<MAX_C;j++){
                        if(clients[j].sock_fd>0 && clients[j].sock_fd !=sockfd)
                            if(send(clients[j].sock_fd,&NEW,sizeof(RECIEVE),0)==-1)
                                perror("send");
                    }

                } else { //bad nick - disconnect
                    strcpy(NEW.nick,"server");
                    strcpy(NEW.message,"*** your nick is already exists\n");
                    send(newsockfd,&NEW,sizeof(NEW),MSG_DONTWAIT);
                    shutdown(newsockfd,2);
                    close(newsockfd);
                    continue;
                }
            }
		    
	    }
    
        printf("send<=>recieve\n");
        for(i=0;i<MAX_C;i++){ //send<=>recieve

            if(FD_ISSET(clients[i].sock_fd, &readfds)){
                if((recvd=recv(clients[i].sock_fd,&RECIEVE,sizeof(RECIEVE),0))<=0){ //shutdown
                    if(recvd<=0){
                        printf("### socket %s disconnected\n",clients[i].nick);
                        shutdown(clients[i].sock_fd,2);
                        close(clients[i].sock_fd);
                        clients[i].sock_fd=0;
                        memset(clients[i].nick,'\0',sizeof(clients[i].nick));
                        memset(clients[i].privates,'\0',sizeof(clients[i].privates));
                        CUR_C--;
                    } /*else {
                        perror("recv");
                    }*/
                    
                } else { //sending
                    printf("recieved\n");
                    printf("printf 1");
                    memset(CMD.message,'\0',sizeof(CMD.message));
                    memset(CMD.nick,'\0',sizeof(CMD.nick));
                    printf("printf 2");
                    if (RECIEVE.message[0]=='\\' && RECIEVE.message[1]=='q' && RECIEVE.message[2]=='u' && RECIEVE.message[3]=='i' && RECIEVE.message[4]=='t' && RECIEVE.message[5]==' ') {

                        strcpy(CMD.nick,"server");
                        strcpy(CMD.message,"*** User ");
                        strcat(CMD.message,clients[i].nick);
                        strcat(CMD.message," left us with message:");
                        strcat(CMD.message,RECIEVE.message+5);
                        for(j=0;j<MAX_C;j++){
                            if(clients[j].sock_fd>0 && j!=i && clients[j].sock_fd !=sockfd)
                                if(send(clients[j].sock_fd,&CMD,sizeof(CMD),0)==-1)
                                    perror("send 1");
                        }
                    printf("printf 3");
                    } else if(RECIEVE.message[0]=='\\' && RECIEVE.message[1]=='u' && RECIEVE.message[2]=='s' && RECIEVE.message[3]=='e' && RECIEVE.message[4]=='r' && RECIEVE.message[5]=='s' && RECIEVE.message[6]=='\n') {
                        
                        strcpy(CMD.nick,"server");
                        strcpy(CMD.message,"*** Users online: ");
                        for(j=0;j<MAX_C;j++){
                            if(clients[j].sock_fd){
                                strcat(CMD.message,clients[j].nick);
                                strcat(CMD.message,"; ");
                            }
                        }
                        strcat(CMD.message,"\n");
                        if(send(clients[i].sock_fd,&CMD,sizeof(CMD),0)==-1)
                            perror("send 2");
                    printf("printf 4");
                    } else if(RECIEVE.message[0]=='\\' && RECIEVE.message[1]=='h' && RECIEVE.message[2]=='e' && RECIEVE.message[3]=='l' && RECIEVE.message[4]=='p' && RECIEVE.message[5]=='\n'){
                        strcpy(CMD.nick,"server");
                        strcpy(CMD.message,"*** Commands to use:\n\\help - you are here\n\\users - see users online\n\\quit <message> - quit chat with message\n\\private <nickname> <message> - send private <message> to <nickname>\n\\privates - see private users list\n");
                        if(send(clients[i].sock_fd,&CMD,sizeof(CMD),0)==-1)
                            perror("send 3");
                    } else if(RECIEVE.message[0]=='\\' && RECIEVE.message[1]=='p' && RECIEVE.message[2]=='r' && RECIEVE.message[3]=='i' && RECIEVE.message[4]=='v' && RECIEVE.message[5]=='a' && RECIEVE.message[6]=='t' && RECIEVE.message[7]=='e' && RECIEVE.message[8]==' '){
                        for(j=9;RECIEVE.message[j]!=' ' && j<MESLEN;j++);
                        if(j>(NICKLEN+9) || j==MESLEN){ //The command is wrong - just a message
                            for(j=0;j<MAX_C;j++){
                                if(clients[j].sock_fd>0 && j!=i && clients[j].sock_fd !=sockfd)
                                    if(send(clients[j].sock_fd,&RECIEVE,sizeof(RECIEVE),0)==-1)
                                        perror("send 4");
                                }
                        } else { //The command is right
                            printf("printf 5");
                            for(k=9;k<j;k++){CMD.nick[k-9]=RECIEVE.message[k];}
                            k=0;
                            while(k<MAX_C && strcmp(clients[k].nick,CMD.nick)){k++;}
                            if(k==MAX_C || !strcmp(RECIEVE.nick,CMD.nick)){
                                if(!strcmp(RECIEVE.nick,CMD.nick)) flag=1; else flag=0; //user offline
                                memset(CMD.nick,'\0',sizeof(CMD.nick));
                                strcpy(CMD.nick,"server");
                                if(flag){
                                    strcpy(CMD.message,"*** You can't send private message to yourself\n");
                                } else strcpy(CMD.message,"*** This user is offline\n");
                                if(send(clients[i].sock_fd,&CMD,sizeof(CMD),0)==-1)
                                    perror("send 5");
                            } else{ //user online -send
                                printf("printf 6");
                                memset(CMD.nick,'\0',sizeof(CMD.nick));
                                strcpy(CMD.nick,RECIEVE.nick);
                                strcpy(CMD.message,"*");
                                strcat(CMD.message,RECIEVE.message+j);
                                if(send(clients[k].sock_fd,&CMD,sizeof(CMD),0)==-1)
                                    perror("send 6");
                                strcat(clients[i].privates,clients[k].nick);
                                strcat(clients[i].privates,"; ");
                                printf("printf 7");
                            }
                        }


                    printf("printf 8");
                    } else if(RECIEVE.message[0]=='\\' && RECIEVE.message[1]=='p' && RECIEVE.message[2]=='r' && RECIEVE.message[3]=='i' && RECIEVE.message[4]=='v' && RECIEVE.message[5]=='a' && RECIEVE.message[6]=='t' && RECIEVE.message[7]=='e' && RECIEVE.message[8]=='s' && RECIEVE.message[9]=='\n'){
                        strcpy(CMD.nick,"server");
                        
                        if(clients[i].privates[0]=='\0'){
                            strcpy(CMD.message,"*** You hadn't sent any private message yet");
                        } else {
                        strcpy(CMD.message,"*** You've sent private messages to these users: ");
                        strcat(CMD.message,clients[i].privates);
                        }
                        strcat(CMD.message,"\n");
                        if(send(clients[i].sock_fd,&CMD,sizeof(CMD),0)==-1)
                            perror("send 3");
                    printf("printf 9");
                    } else {
                        printf("just mes\n");
                        for(j=0;j<MAX_C;j++){
                            if(clients[j].sock_fd>0 && j!=i && clients[j].sock_fd !=sockfd)
                                if(send(clients[j].sock_fd,&RECIEVE,sizeof(RECIEVE),0)==-1)
                                    perror("send 7");
                        
                        }
                        printf("printf 10");
                    }
                }
            }

        }


    }

    return 0;
}