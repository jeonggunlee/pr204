#include "common_impl.h"

int init_socket(int type)
{	
	int fd;
	int yes = 1;
	/* creation de la socket */
	fd = socket(AF_INET, type, 0);

	/* verification de la validite de la socket */
	if (fd == -1)
		ERROR_EXIT("Socket error");

	/* socket option pour le probleme "already in use" */
	if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1)
		ERROR_EXIT("Error setting socket options");

	return fd;
}

int creer_socket(int type, int * port_num) 
{
	int fd = init_socket(type);
	struct sockaddr_in server_addr;

	/* initialisation du serveur */
    memset(&server_addr, 0, sizeof(struct sockaddr_in));
    server_addr.sin_port = htons(*port_num);
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
	
    /* attachement de la nouvelle socket */
	while(bind(fd, (struct sockaddr *)&server_addr, sizeof(struct sockaddr_in)) == -1)
	{
		/* autre erreur que le port */
		if (errno != EADDRINUSE)
			ERROR_EXIT("Bind error");

		/* attribution d'un nouveau port */
		(*port_num)++;
		server_addr.sin_port = htons(*port_num);
	}

	return fd;
}

void * arguments(int fd, const char * type)
{
	proc_args_t * args = malloc(4 + sizeof(int));
	args->fd = fd;
	args->type = type;
	return args;
}

/* Vous pouvez ecrire ici toutes les fonctions */
/* qui pourraient etre utilisees par le lanceur */
/* et le processus intermediaire. N'oubliez pas */
/* de declarer le prototype de ces nouvelles */
/* fonctions dans common_impl.h */