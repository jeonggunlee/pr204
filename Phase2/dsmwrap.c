#include "common_impl.h"

void getfullhostname(char buf[LENGTH])
{
	struct hostent * host;

	memset(buf, 0, LENGTH);
	gethostname(buf, LENGTH);

	host = gethostbyname(buf);
	memset(buf, 0, LENGTH);
	strcpy(buf, host->h_name);
}

int main(int argc, char ** argv)
{
	int i;
	int fd;	// socket temporaire
	int index;
	int port;
	int port_dsmexec;
	char buf[LENGTH];
	char * hostname_dsmexec;
	char ** newargv;
	struct sockaddr_in sockaddr_dsmexec;

	// recuperation de l'index
	index = atoi(argv[argc-1]);

	// recuperation du port de dsmexec
	port_dsmexec = atoi(argv[argc-3]);

	// recuperation de l'adresse ip de dsmexec
	hostname_dsmexec = string_copy(argv[argc-2]);

	newargv = malloc((argc-2) * sizeof(char *));

	for (i = 0; i < argc-4; i++)
		newargv[i] = string_copy(argv[i+1]);

	newargv[argc-3] = NULL;

	// creation d'une socket pour se connecter au
	// au lanceur et envoyer/recevoir les infos
	// necessaires pour la phase dsm_init
	get_addr_info(&sockaddr_dsmexec, hostname_dsmexec, port_dsmexec);

	fd = init_socket(SOCK_STREAM);
	do_connect(fd, sockaddr_dsmexec);
	
	memset(buf, '\0', LENGTH);

	// Recuperation du nom de la machine dans le buffer
	getfullhostname(buf);
	buf[strlen(buf)] = '\n';

	// Envoi du nom de machine au lanceur
	handle_client_message(fd, buf);

	// Envoi du pid au lanceur
	int_in_buf(getpid(), buf);
	handle_client_message(fd, buf);

	// Creation de la socket d'ecoute pour les
	// connexions avec les autres processus dsm
	creer_socket(SOCK_STREAM, &port);

	// Envoi du numero de port au lanceur
	// pour qu'il le propage a tous les autres
	// processus dsm
	int_in_buf(port, buf);
	handle_client_message(fd, buf);

	// Renvoi de l'index de la machine
	int_in_buf(index, buf);
	handle_client_message(fd, buf);

	sleep(1);

	// on execute la bonne commande
	execvp(newargv[0], newargv);

	return 0;
}
