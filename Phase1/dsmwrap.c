#include "common_impl.h"

int main(int argc, char ** argv)
{
	int client_sock;
	int sock;
	int port_dsmexec = atoi(argv[argc-1]);
	int mon_port;

	/* processus intermediaire pour "nettoyer" */
	/* la liste des arguments qu'on va passer */
	/* a la commande a executer vraiment */

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

	/* on execute la bonne commande */
	return 0;
}
