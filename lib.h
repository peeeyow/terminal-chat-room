#ifndef __LIB_H__
#define __LIB_H__
// variables / structs / other globally used data
typedef struct client client_t;
struct client {
  char user_name[17];
  int socket_name;
};
typedef struct message_t message_t;
struct message_t {
  char command[10];
  char name[17];
  char *message;
  int delay;
};
// functions
message_t parser();
int name_checking();
int print_time();
int check_message();
void handsegv();
char *myfgets();

#endif
