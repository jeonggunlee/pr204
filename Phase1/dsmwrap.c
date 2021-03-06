#include "common_impl.h"

int main(int argc, char ** argv)
{
	int i;
	int fd;
	int port;
	char buf[LENGTH];
	struct sockaddr_in sockaddr_dsmexec;

	memset(buf, '\0', LENGTH);

	// recuperation du port de dsmexec
	int port_dsmexec = atoi(argv[argc-2]);

	// recuperation de l'adresse ip de dsmexec
	char * hostname_dsmexec = string_copy(argv[argc-1]);

	char ** newargv = malloc((argc-3) * sizeof(char *));

	for (i = 0; i < argc-3; i++)
		newargv[i] = string_copy(argv[i+1]);
	
	newargv[argc-3] = NULL;

	// creation d'une socket pour se connecter au
	// au lanceur et envoyer/recevoir les infos
	// necessaires pour la phase dsm_init
	get_addr_info(&sockaddr_dsmexec, hostname_dsmexec, port_dsmexec);

	fd = init_socket(SOCK_STREAM);
	do_connect(fd, sockaddr_dsmexec);

	// Recuperation du nom de la machine dans le buffer
	gethostname(buf, LENGTH);
	buf[strlen(buf)] = '\n';

	// Envoi du nom de machine au lanceur
	handle_client_message(fd, buf);

	// Envoi du pid au lanceur
	memset(buf, '\0', LENGTH);
	sprintf(buf, "%d\n", getpid());
	handle_client_message(fd, buf);

	// Creation de la socket d'ecoute pour les
	// connexions avec les autres processus dsm
	creer_socket(SOCK_STREAM, &port);

	// Envoi du numero de port au lanceur
	// pour qu'il le propage à tous les autres
	// processus dsm
	memset(buf, '\0', LENGTH);
	sprintf(buf, "%d\n", port);
	handle_client_message(fd, buf);

	// on execute la bonne commande
	execvp(newargv[0], newargv);

	return 0;
}
