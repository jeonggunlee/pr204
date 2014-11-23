#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include <assert.h>

/* autres includes (eventuellement) */

#define ERROR_EXIT(str) {perror(str);exit(EXIT_FAILURE);}
#define LENGTH 256

/* definition du type des infos */
/* de connexion des processus dsm */
struct dsm_proc_conn
{
   int rank;
   /* a completer */
};

typedef struct dsm_proc_conn dsm_proc_conn_t; 

/* definition du type des infos */
/* d'identification des processus dsm */
struct dsm_proc
{
	int rank;
	pid_t pid;
	char * name;
	dsm_proc_conn_t connect_info;
};

typedef struct dsm_proc dsm_proc_t;

int creer_socket(int type, int *port_num);

void error(const char *msg);