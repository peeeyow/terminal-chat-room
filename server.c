#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/signal.h>
#include <netinet/in.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <ctype.h>
#include <time.h>
#include <sys/time.h>
#include "lib.h"

int main(int argc, char *argv[]) {
  char address[16], name[17], *buff;
  int server_fd, client_fd, test_fd, port, error_checking, i, j, size,
      client_count = 0, fd_no_max, serving,
      opt = 1; // server_fd = server itself, client
  struct sockaddr_in server_addr,
      client_addr; // struct for the server address info, and the clients
                   // address info
  message_t parse;
  pid_t stall;
  fd_set core, temp; // general file descriptor list, temp FD list for select()
  client_t client_list[4];
  if (argc != 3) {
    puts("Wrong commandline input(s)");
    exit(1);
  }
  signal(SIGSEGV, handsegv);
  sscanf(argv[2], "%d", &port);
  puts("SERVER: Waiting for connections...");
  server_fd =
      socket(AF_INET, SOCK_STREAM, 0); // creation of server's file descriptor
  if (server_fd < 0) {
    perror("Failed Creating Server");
    exit(1);
  }
  if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, (char *)&opt,
                 sizeof(opt)) < 0) {
    perror("Error setsockopt()");
    exit(1);
  }
  server_addr.sin_family = AF_INET; // assigning address for server
  server_addr.sin_addr.s_addr = inet_addr(argv[1]);
  server_addr.sin_port = htons(port);
  if (bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) <
      0) {
    perror("Binding address to Server File Descriptor failed!");
    exit(1);
  }
  error_checking = listen(server_fd, 4); // getting ready to receive a client
  if (error_checking < 0) {
    perror("Error listen()");
  }
  memset(client_list, 0, sizeof(client_list));
  FD_ZERO(&core);
  FD_ZERO(&temp);
  FD_SET(server_fd, &core);
  fd_no_max = server_fd;
  for (;;) { // start
    temp = core;
    error_checking = select(fd_no_max + 1, &temp, NULL, NULL, NULL);
    if (error_checking == -1) {
      perror("Error select()");
      exit(4);
    }
    for (serving = 0; serving <= fd_no_max; serving++) {
      if (FD_ISSET(serving,
                   &temp)) { // searches for fd that are members of the fd_set
        if (serving == server_fd) { // accepts new client until 4
          size = sizeof(client_addr);
          if (client_count < 4) {
            client_fd =
                accept(server_fd, (struct sockaddr *)&client_addr, &size);
            if (client_fd < 0) {
              perror("SERVER: Error Accepting Client!");
              exit(1);
            } else {
              inet_ntop(AF_INET, &(client_addr.sin_addr), address,
                        sizeof(address));
              printf("SERVER: Got a connection from %s\n", address);
              puts("SERVER: Sending Request for name of new client.");
              error_checking =
                  send(client_fd, "SERVER :Please Enter your Username:",
                       sizeof("SERVER: Please Enter your Username:"), 0);
              if (error_checking == -1) {
                perror("Error send()");
                exit(1);
              }
              for (;;) { // ask for username checks for duplicates
                bzero(name, sizeof(name));
                error_checking = recv(client_fd, name, sizeof(name), 0);
                if (error_checking == 0) {
                  puts("SERVER: Unexpected Disconnection from unnamed client");
                  FD_CLR(client_fd, &core);
                  close(client_fd);
                  break;
                } else if (error_checking == -1) {
                  perror("Error recv()");
                  FD_CLR(client_fd, &core);
                  close(client_fd);
                  break;
                }
                for (i = 0, j = 0; i < client_count; i++) {
                  if (strcmp(client_list[i].user_name, name) == 0) {
                    j = 1;
                  }
                }
                if (j == 0) {
                  error_checking = send(client_fd, "Good", sizeof("Good"), 0);
                  if (error_checking == -1) {
                    perror("Error send()");
                    exit(1);
                  }
                  for (i = 0; i < 4; i++) {
                    if (client_list[i].socket_name == 0) {
                      strcpy(client_list[client_count].user_name, name);
                      client_list[client_count].socket_name = client_fd;
                      client_count++; // increments the count because a client
                                      // has been added
                      FD_SET(client_fd,
                             &core); // adds the new client to the fd_set
                      if (client_fd >
                          fd_no_max) { // renews the highest fd number
                        fd_no_max = client_fd;
                      }
                      break;
                    }
                  }
                  printf("SERVER: New client name is %s\n", name);
                  bzero(name, sizeof(name));
                  break;
                } else {
                  error_checking =
                      send(client_fd, "Duplicate Username :(.\nEnter Again.",
                           sizeof("Duplicate Username :(.\nEnter Again."), 0);
                  if (error_checking == -1) {
                    perror("Error send()");
                    exit(1);
                  }
                }
              }
            }
            for (i = 0; i < 4; i++) {
              printf("SERVER: Socket # %d, Username: %s\n",
                     client_list[i].socket_name, client_list[i].user_name);
            }
            printf("SERVER: There are %d client(s) connected\n", client_count);
          } else {
            client_fd =
                accept(server_fd, (struct sockaddr *)&client_addr, &size);
            close(client_fd);
          }
        } else {
          if ((error_checking = recv(serving, &size, sizeof(size), 0)) <= 0) {
            if (error_checking == 0) { // DISCONNECTED CLIENTS
              for (i = 0; i < 4;
                   i++) { // search for the name of the disconnected client
                if (client_list[i].socket_name == serving) {
                  printf("SERVER: %s Disconnected :(.\n",
                         client_list[i].user_name);
                  memset(&client_list[i], 0, sizeof(client_list[i]));
                  client_count--;
                  close(serving);
                  FD_CLR(serving, &core);
                  break;
                }
              }
            } else if (error_checking == -1) {
              perror("Error recv()");
            }
            printf("SERVER: There are %d client(s) connected\n", client_count);
          } else { // allocation for buffer then receiving the real string;
            for (i = 0; i < client_count;
                 i++) { // search for the name of the disconnected client
              if (client_list[i].socket_name == serving) {
                strcpy(name, client_list[i].user_name);
                break;
              }
            }
            buff = (char *)malloc(size * sizeof(char));
            bzero(buff, strlen(buff));
            if ((error_checking = recv(serving, buff, size, 0)) <= 0) {
              if (error_checking == 0) {
                for (i = 0; i < client_count;
                     i++) { // search for the name of the disconnected client
                  if (client_list[i].socket_name == serving) {
                    printf("SERVER: %s Disconnected :(.\n",
                           client_list[i].user_name);
                  }
                }
              } else if (error_checking == -1) {
                perror("Error recv()");
              }
              memset(&client_list[i], 0, sizeof(client_list[i]));
              for (; i < client_count; i++) {
                client_list[i] = client_list[i + 1];
                memset(&client_list[i + 1], 0, sizeof(client_list[i]));
              }
              client_count--;
              close(serving);
              FD_CLR(serving, &core);
            } else { // send message to all or specific
              printf("SERVER: %s", name);
              printtime();
              puts(buff);
              parse = parser(buff, size);
              if (strcmp(parse.command, "/pm") == 0) { // pm
                for (i = 0, j = 0; i < client_count; i++) {
                  if ((strcmp(parse.name, client_list[i].user_name) == 0) &&
                      (client_list[i].socket_name != serving)) {
                    j = 1;
                    error_checking =
                        send(client_list[i].socket_name, name, sizeof(size), 0);
                    if (error_checking == -1) {
                      perror("Error errno");
                      exit(1);
                    }
                    size = strlen(parse.message) + 1;
                    error_checking = send(client_list[i].socket_name, &size,
                                          sizeof(size), 0);
                    if (error_checking == -1) {
                      perror("Error errno");
                      exit(1);
                    }
                    error_checking = send(client_list[i].socket_name,
                                          parse.message, size, 0);
                    if (error_checking == -1) {
                      perror("Error errno");
                      exit(1);
                    }
                    break;
                  }
                }
                if (j == 0) {
                  bzero(name, sizeof(name));
                  error_checking = send(serving, name, sizeof(size), 0);
                  if (error_checking == -1) {
                    perror("Error errno");
                    exit(1);
                  }
                  strcpy(parse.message, "SERVER: Name not found");
                  size = strlen(parse.message) + 1;
                  error_checking = send(serving, &size, sizeof(size), 0);
                  if (error_checking == -1) {
                    perror("Error errno");
                    exit(1);
                  }
                  error_checking = send(serving, parse.message, size, 0);
                  if (error_checking == -1) {
                    perror("Error errno");
                    exit(1);
                  }
                }
              } else if (strcmp(parse.command, "/delay") == 0) { // pm
                stall = fork();
                if (stall == 0) {
                  sleep(parse.delay);
                  for (i = 0; i < 4; i++) {
                    if (FD_ISSET(client_list[i].socket_name, &core)) {
                      if ((client_list[i].socket_name != server_fd) &&
                          (client_list[i].socket_name !=
                           serving)) { // must not send to self and server;
                        if (send(client_list[i].socket_name, name, sizeof(name),
                                 0) == -1) {
                          perror("Error send()");
                        }
                        size = strlen(parse.message) + 1;
                        if (send(client_list[i].socket_name, &size,
                                 sizeof(size), 0) == -1) {
                          perror("Error send()");
                        }
                        if (send(client_list[i].socket_name, parse.message,
                                 size, 0) == -1) {
                          perror("Error send()");
                        }
                      }
                    }
                  }
                  exit(1);
                }
              } else if (strcmp(parse.command, "/delaypm") == 0) { // pm
                stall = fork();
                if (stall == 0) {
                  sleep(parse.delay);
                  for (i = 0; i < client_count; i++) {
                    if ((strcmp(parse.name, client_list[i].user_name) == 0) &&
                        (client_list[i].socket_name != serving)) {
                      error_checking = send(client_list[i].socket_name, name,
                                            sizeof(size), 0);
                      if (error_checking == -1) {
                        perror("Error errno");
                        exit(1);
                      }
                      size = strlen(parse.message) + 1;
                      error_checking = send(client_list[i].socket_name, &size,
                                            sizeof(size), 0);
                      if (error_checking == -1) {
                        perror("Error errno");
                        exit(1);
                      }
                      error_checking = send(client_list[i].socket_name,
                                            parse.message, size, 0);
                      if (error_checking == -1) {
                        perror("Error errno");
                        exit(1);
                      }
                      break;
                    }
                  }
                }
              } else if (strcmp(parse.command, "/chname") == 0) {
                if (name_checking(parse.name) == 0) {
                  for (i = 0, j = 0; i < 4; i++) {
                    if (strcmp(client_list[i].user_name, parse.name) == 0) {
                      j = 1;
                    }
                  }
                  if (j == 0) {
                    for (i = 0; i < 4; i++) {
                      if (client_list[i].socket_name == serving) {
                        break;
                      }
                    }
                    strcpy(client_list[i].user_name, parse.name);
                    bzero(name, sizeof(name));
                    if (send(serving, name, sizeof(name), 0) == -1) {
                      perror("Error send()");
                    }
                    size = sizeof("SERVER: Username change was succesful.");
                    if (send(serving, &size, sizeof(size), 0) == -1) {
                      perror("Error send()");
                    }
                    if (send(serving, "SERVER: Username change was succesful.",
                             size, 0) == -1) {
                      perror("Error send()");
                    }
                  } else if (j == 1) {
                    strcpy(client_list[i].user_name, parse.name);
                    bzero(name, sizeof(name));
                    if (send(serving, name, sizeof(name), 0) == -1) {
                      perror("Error send()");
                    }
                    size =
                        sizeof("SERVER: Username change was NOT succesful.") +
                        1;
                    if (send(serving, &size, sizeof(size), 0) == -1) {
                      perror("Error send()");
                    }
                    if (send(serving,
                             "SERVER: Username change was NOT succesful.", size,
                             0) == -1) {
                      perror("Error send()");
                    }
                  }
                }
              } else {
                for (i = 0; i < 4; i++) {
                  if (FD_ISSET(client_list[i].socket_name, &core)) {
                    if ((client_list[i].socket_name != server_fd) &&
                        (client_list[i].socket_name !=
                         serving)) { // must not send to self and server;
                      if (send(client_list[i].socket_name, name, sizeof(name),
                               0) == -1) {
                        perror("Error send()");
                      }
                      size = strlen(parse.message) + 1;
                      if (send(client_list[i].socket_name, &size, sizeof(size),
                               0) == -1) {
                        perror("Error send()");
                      }
                      if (send(client_list[i].socket_name, parse.message, size,
                               0) == -1) {
                        perror("Error send()");
                      }
                    }
                  }
                }
              }
            }
          }
        }
      }
    }
  }
  return 0;
}
