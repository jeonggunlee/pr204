#include "common_impl.h"

int main(int argc, char ** argv)
{
	int i;
	int fd;
	char buf[LENGTH];
	struct sockaddr_in sockaddr_dsmexec;

	memset(buf, '\0', LENGTH);

	// recuperation du nombre de processus
	int num_procs = atoi(argv[argc-1]);
	// recuperation du port de dsmexec
	int port_dsmexec = atoi(argv[argc-3]);

	// recuperation de l'adresse ip de dsmexec
	char * hostname_dsmexec = string_copy(argv[argc-2]);
	char * machines[num_procs];

	char ** newargv = malloc((argc-num_procs-3) * sizeof(char *));

	// recuperation du nom des machines
	for (i = 0; i < num_procs; i++)
		machines[i] = string_copy(argv[argc-3-num_procs+i]);

	for (i = 0; i < argc-num_procs-3-1; i++)
		newargv[i] = string_copy(argv[i+1]);
	
	newargv[argc-num_procs-3] = NULL;

	// creation d'une socket pour se connecter au
	// au lanceur et envoyer/recevoir les infos
	// necessaires pour la phase dsm_init
	get_addr_info(&sockaddr_dsmexec, hostname_dsmexec, port_dsmexec);

	fd = init_socket(SOCK_STREAM);
	do_connect(fd, sockaddr_dsmexec);

	// Recuperation du nom de la machine dans le buffer
	gethostname(buf, LENGTH);

	// Envoi du nom de machine au lanceur
	handle_client_message(fd, buf);

	// Envoi du pid au lanceur
	memset(buf, '\0', LENGTH);
	sprintf(buf, "%d", getpid());
	handle_client_message(fd, buf);

	// Creation de la socket d'ecoute pour les
	// connexions avec les autres processus dsm

	// Envoi du numero de port au lanceur
	// pour qu'il le propage à tous les autres
	// processus dsm

	// on execute la bonne commande
	// execvp(newargv[0], newargv);

	return 0;
}
