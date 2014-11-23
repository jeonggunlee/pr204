#include "common_impl.h"

int creer_socket(int prop, int * port_num) 
{
	int fd = 0;
	int yes = 1;
	struct sockaddr_in server_addr;

	/* fonction de creation et d'attachement */
	/* d'une nouvelle socket */
	/* renvoie le numero de descripteur */
	/* et modifie le parametre port_num */

	/* do_socket() */
	fd = socket(AF_INET, prop, 0);
	if (fd == -1)
		error("Socket error");

	if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1)
		error("Error setting socket options");

	/* init_serv_addr() */
	memset(&server_addr, 0, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = ntohs(*port_num);
	server_addr.sin_addr.s_addr = INADDR_ANY;

	/* do_bind() */
	if (bind(fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1)
        error("Bind error");

	return fd;
}

/* Vous pouvez ecrire ici toutes les fonctions */
/* qui pourraient etre utilisees par le lanceur */
/* et le processus intermediaire. N'oubliez pas */
/* de declarer le prototype de ces nouvelles */
/* fonctions dans common_impl.h */