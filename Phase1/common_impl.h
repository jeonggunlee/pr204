#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include <math.h>
#include <pthread.h>
#include <netdb.h>

/* autres includes (eventuellement) */
#define LENGTH			512
#define DSMWRAP_PATH	"~/PR204/Phase1/bin/dsmwrap"
#define ERROR_EXIT(str)	{perror(str);exit(EXIT_FAILURE);}

/* definition du type des infos */
/* de connexion des processus dsm */
struct dsm_proc_conn
{
	int rank;
	int port;
	char * hostname;
};
typedef struct dsm_proc_conn dsm_proc_conn_t; 

/* definition du type des infos */
/* d'identification des processus dsm */
struct dsm_proc
{   
	pid_t pid;
	dsm_proc_conn_t connect_info;
};
typedef struct dsm_proc dsm_proc_t;

struct proc_args
{
	int fd;
	char * type;
	char * machine;
	int rank;
};
typedef struct proc_args proc_args_t;

int count_digits(int number);
char * string_copy(char * src);
char * int_copy(int number);
int do_accept(int sock, struct sockaddr * client_addr, socklen_t * client_size);
void read_line(int fd, char buf[LENGTH]);
void * arguments(int fd, char * type, dsm_proc_t * proc);
void get_addr_info(struct sockaddr_in * server, char * address, int port);
void do_connect(int sock, struct sockaddr_in addr);
void handle_client_message(int fd, char * msg);
void receive(int fd, char * buf);
int init_socket(int type);
int creer_socket(int type, int * port_num);
