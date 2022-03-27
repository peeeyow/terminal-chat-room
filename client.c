#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <netdb.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <sys/time.h> 
#include "LUMEN_MENDOZA_MP1_lib.h"

int main(int argc, char *argv[]){
    int client, i, j, len, port, error, maxfd, size, settings;
    char *reads, *writes, address[16], server_default[128], name[17], *buff, myname[17];
    message_t parse;
    fd_set rd;
    struct sockaddr_in addr;//, and the clients address info
    struct hostent *server = (struct hostent *)malloc(sizeof(struct hostent));
    if(argc != 3){
        puts("Wrong commandline input(s)");
        exit(1);
    }
    sscanf(argv[2], "%d", &port);
    client = socket(AF_INET, SOCK_STREAM, 0);// creation of client's file descriptor
    if(client < 0){
        perror("Failed Creating Client!");
        exit(1);
    }
    memset(server, 0, sizeof(server));
    server = gethostbyname(argv[1]);
    if(server == NULL){
        perror("gethostbyname error");
    }
    addr.sin_family = AF_INET;
    memcpy(&addr.sin_addr, server->h_addr, server->h_length);
    addr.sin_port = htons(port);
    error = connect(client, (struct sockaddr *)&addr, sizeof(addr));
    if(error < 0){
        perror("Failed to connect to a server\nServer busy");
        exit(1);
    }
    inet_ntop(AF_INET, &(addr.sin_addr), address, sizeof(address));
    printf("Client: connecting to %s\n", address);
    //initialization of the name/ conecction
    error = recv(client, server_default, sizeof(server_default), 0);
    if(error < 0){
        perror("Failed to receive message to server");
        close(client);
        exit(1);
    }
    else if(error == 0){
        puts("Server Disconnected, Client Closed.");
        close(client);
        exit(1);
    }
    puts(server_default);
    bzero(server_default, sizeof(server_default));
    for(;;){// Register name to the server
        for(;;){
            buff = myfgets();
            if(name_checking(buff) == 0){
                break;
            }
            else{
                puts("SERVER : Invalid username!");
                puts("SERVER : Enter again:");
            }
        }
        strcpy(server_default, buff);
        strcpy(myname, buff);
        send(client, server_default, strlen(server_default), 0);
        bzero(server_default, strlen(server_default));
        recv(client, server_default, sizeof(server_default), 0);
        if(strcmp("Good", server_default) == 0){
            puts("SERVER: Username is successfully registered in the server.");
            free(buff);
            break;
        }
        else {
            puts(server_default);
            bzero(server_default, strlen(server_default));
        }
    }
    settings = fcntl(STDIN_FILENO, F_GETFL, 0);
    fcntl(STDIN_FILENO, F_SETFL, settings | O_NONBLOCK);
    maxfd = client;
    for(;;){
        FD_ZERO(&rd);
        FD_SET(STDIN_FILENO, &rd);
        FD_SET(client, &rd);
        error = select(maxfd + 1, &rd, NULL, NULL, NULL);
        if(error < 0){
            perror("select()");
            exit(1);
        }
        if(FD_ISSET(client, &rd)){
            bzero(name, sizeof(name));
            error = recv(client, name, sizeof(name), 0);
            if(error < 0){
                perror("recv()");
                close(client);
                exit(1);
            }
            else if(error == 0){
                puts("Server Disconnected.");
                close(client);
                exit(1);
            }
			error = recv(client, &size, sizeof(size), 0);
            if(error < 0){
                perror("recv()");
                close(client);
                exit(1);
            }
            else if(error == 0){
                puts("Server Disconnected.");
                close(client);
                exit(1);
            }
            reads = (char *)malloc(size*sizeof(char));
            bzero(reads, sizeof(reads));
            error = recv(client, reads, size, 0);
            if(error < 0){
                perror("recv()");
                close(client);
                exit(1);
            }
            else if(error == 0){
                puts("Server Disconnected.");
                close(client);
                exit(1);
            }
            else{
                printf("%s", name);
                printtime();
                puts(reads);
                bzero(name, strlen(name));
                free(reads);
            }
        }
        else if(FD_ISSET(STDIN_FILENO, &rd)){
            writes = myfgets();
            size = strlen(writes) + 1;
            parse = parser(writes, size);
            if(strcmp("/delay", parse.command) == 0){
                if((parse.delay > 0) && (check_message(parse.message) == 0)){
                    error = send(client, &size, sizeof(size), 0);
                    if(error < 0){
                        perror("send()");
                        exit(1);
                    }
                    error = send(client, writes, size, 0);
                    if(error < 0){
                        perror("send()");
                        exit(1);
                    }
                    printf("%s", myname);
                    printtime();
                    puts(writes);
                    free(writes);
                }
                else{
                    puts("SERVER: Message was not sent!.");
                }
            }
            else if(strcmp("/pm", parse.command) == 0){
                if((parse.delay == 0) && (check_message(parse.message) == 0) && (strlen(parse.name) != 0)){
                    error = send(client, &size, sizeof(size), 0);
                    if(error < 0){
                        perror("send()");
                        exit(1);
                    }
                    error = send(client, writes, size, 0);
                    if(error < 0){
                        perror("send()");
                        exit(1);
                    }
                    printf("%s", myname);
                    printtime();
                    puts(writes);
                    free(writes);
                }
                else{
                    puts("SERVER: Message was not sent!.");
                }
            }
            else if(strcmp("/delaypm", parse.command) == 0){
                if((parse.delay > 0) && (check_message(parse.message) == 0) && (strlen(parse.name) != 0)){
                    error = send(client, &size, sizeof(size), 0);
                    if(error < 0){
                        perror("send()");
                        exit(1);
                    }
                    error = send(client, writes, size, 0);
                    if(error < 0){
                        perror("send()");
                        exit(1);
                    }
                    printf("%s", myname);
                    printtime();
                    puts(writes);
                    free(writes);
                }
                else{
                    puts("SERVER: Message was not sent!.");
                }
            }
            else if(strcmp("/chname", parse.command) == 0){
                error = send(client, &size, sizeof(size), 0);
                if(error < 0){
                    perror("send()");
                    exit(1);
                }
                error = send(client, writes, size, 0);
                if(error < 0){
                    perror("send()");
                    exit(1);
                }                
                printf("%s", myname);
                printtime();
                puts(writes);
                free(writes);
            }
            else {
                if((parse.delay == 0) && (check_message(parse.message) == 0)){
                    error = send(client, &size, sizeof(size), 0);
                    if(error < 0){
                        perror("send()");
                        exit(1);
                    }
                    error = send(client, writes, size, 0);
                    if(error < 0){
                        perror("send()");
                        exit(1);
                    }
                    printf("%s", myname);
                    printtime();
                    puts(writes);
                    free(writes);
                }
                else{
                    puts("SERVER: Message was not sent!.");
                }
            }
        }
    }
}