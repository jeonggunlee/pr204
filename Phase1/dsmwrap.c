#include "common_impl.h"

int main(int argc, char ** argv)
{
	int i;
	int client_sock;
	int sock;
	int num_procs = atoi(argv[argc-1]);
	int port_launcher = atoi(argv[argc-3]);
	int mon_port;

	char * host = argv[argc-2];
	char * machine_name[num_procs];
	char * arg_exec[argc-num_procs-2];

	/* processus intermediaire pour "nettoyer" */
	/* la liste des arguments qu'on va passer */
	/* a la commande a executer vraiment */

	/* recuperation des noms des machines */
	for (i = 0; i < num_procs; i++)
		machine_name[i] = argv[argc-i-4];

	/* creation d'une socket pour se connecter au */
	/* au lanceur et envoyer/recevoir les infos */
	/* necessaires pour la phase dsm_init */
	client_sock = init_socket(SOCK_STREAM);

	/* Envoi du nom de machine au lanceur */

	/* Envoi du pid au lanceur */

	/* Creation de la socket d'ecoute pour les */
	/* connexions avec les autres processus dsm */
	sock = creer_socket(SOCK_STREAM, &mon_port);

	/* Envoi du numero de port au lanceur */
	/* pour qu'il le propage à tous les autres */
	/* processus dsm */

	/* 1 - recuperation des "vrais" arguments */
	for (i = 1; i < argc-num_procs-3; i++)
		arg_exec[i-1] = argv[i];

	/* 2 - ajout du NULL */
	arg_exec[argc-num_procs-3] = NULL;

	/* on execute la bonne commande */
	execvp(arg_exec[0], arg_exec);


	return 0;
}
