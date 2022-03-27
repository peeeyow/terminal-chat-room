#include "lib.h"
#include <ctype.h>
#include <errno.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

message_t
parser(char *string,
       int size) { // pure parsing checking of validity must be on client
  message_t parsed;
  memset(&parsed, 0, sizeof(parsed));
  char *buff = (char *)malloc(size * sizeof(char)), *checker;
  sscanf(string, "%s", parsed.command);
  if (strcmp(parsed.command, "/pm") == 0) {
    sscanf(string, "%*s %s", parsed.name);
    parsed.message = (char *)malloc(size * sizeof(char));
    checker = strchr(string, '"');
    if (checker != NULL) {
      strcpy(buff, checker);
    }
    if (buff[0] == '"') {
      strcpy(parsed.message, buff + 1);
      if (parsed.message[strlen(parsed.message) - 1] == '"') {
        parsed.message[strlen(parsed.message) - 1] = '\0';
      } else {
        strcpy(parsed.message, "()()");
      }
    }
  } else if (strcmp(parsed.command, "/delay") == 0) {
    sscanf(string, "%*s %d", &(parsed.delay));
    parsed.message = (char *)malloc(
        size * sizeof(char)); // buff and buff2 are now useless can be freed
    checker = strchr(string, '"');
    if (checker != NULL) {
      strcpy(buff, checker);
    }
    if (buff[0] == '"') {
      strcpy(parsed.message, buff + 1);
      if (parsed.message[strlen(parsed.message) - 1] == '"') {
        parsed.message[strlen(parsed.message) - 1] = '\0';
      } else {
        strcpy(parsed.message, "()()");
      }
    }
  } else if (strcmp(parsed.command, "/delaypm") == 0) {
    sscanf(string, "%*s %d", &(parsed.delay));
    sscanf(string, "%*s %*d %s", parsed.name);
    parsed.message = (char *)malloc(
        size * sizeof(char)); // buff and buff2 are now useless can be freed
    checker = strchr(string, '"');
    if (checker != NULL) {
      strcpy(buff, checker);
    }
    if (buff[0] == '"') {
      strcpy(parsed.message, buff + 1);
      if (parsed.message[strlen(parsed.message) - 1] == '"') {
        parsed.message[strlen(parsed.message) - 1] = '\0';
      } else {
        strcpy(parsed.message, "()()");
      }
    }
  } else if (strcmp(parsed.command, "/chname") == 0) {
    sscanf(string, "%*s %s", parsed.name);
  } else {
    parsed.message = (char *)malloc(
        size * sizeof(char)); // buff and buff2 are now useless can be freed
    strcpy(parsed.message, string);
  }
  free(buff);
  return parsed;
}
int check_message(char *string) {
  int i = 0;
  while (string[i] != '\0') { // parsing
    if (!(isalnum(string[i]))) {
      if (!(isspace(string[i]))) {
        if (string[i] != '.') {
          if (string[i] != '!') {
            puts("SERVER: Messages should contain the following characters "
                 "only:");
            puts("SERVER: 0-9, a-z, A-Z, space, period, and exclamation.");
            puts("SERVER: Something is wrong with the input.");
            return -1;
          }
        }
      }
    }
    i++;
  }
  return 0;
}
void handsegv(int flags) {
  perror("SEGV");
  exit(1);
}
char *myfgets() {
  char *string = (char *)malloc(sizeof(char));
  int letter, cletter;
  cletter = 0;
  while ((letter = getchar()) != '\n' && letter != EOF) {
    string[cletter++] = letter;
    string = (char *)realloc(string, (cletter + 1) * sizeof(char));
  }
  string[cletter] = '\0';
  return string;
}
void printtime() {
  time_t raw;
  struct tm *holder;
  char timeprint[32];
  time(&raw);
  holder = localtime(&raw);
  strftime(timeprint, 32, "%a %m/%d/%Y %H:%M:%S", holder);
  printf(" (%s) >  ", timeprint);
  return;
}
int name_checking(char *string) {
  int i = 0;
  if (isdigit(string[0])) {
    puts("Username must start with a letter.");
    return -1;
    ;
  } else if ((strlen(string) < 4) || (strlen(string) > 16)) {
    puts("Username must be 4-16 characters only.");
    return -1;
  } else {
    i = 0;
    while (string[i] != '\0') { // parsing
      if (!(isalnum(string[i]))) {
        puts("Username must be alphanumeric only.");
        return -1;
      }
      i++;
    }
  }
  return 0;
}
