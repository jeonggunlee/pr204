#include "common_impl.h"

/* variables globales */

/* un tableau gerant les infos d'identification */
/* des processus dsm */
dsm_proc_t *proc_array = NULL; 

/* le nombre de processus effectivement crees */
volatile int num_procs_creat = 0;

void usage(void)
{
	fprintf(stdout,"Usage : dsmexec machine_file executable arg1 arg2 ...\n");
	fflush(stdout);
	exit(EXIT_FAILURE);
}

void error(const char *msg)
{
    perror(msg);
    exit(1);
}

void sigchld_handler(int sig)
{
	 /* on traite les fils qui se terminent */
	 /* pour eviter les zombies */
}

int do_socket(int domain, int type, int protocol)
{
	int yes = 1;

	//create the socket
	int fd = socket(domain, type, protocol);

	//check for socket validity
	if (fd == -1)
		error("Socket error");

	//set socket option, to prevent "already in use" issue when rebooting the server right on
	if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1)
		error("Error setting socket options");
	
	return fd;
}

void init_serv_addr(struct sockaddr_in * server_addr, int port)
{
    memset(server_addr, 0, sizeof(*server_addr));
    server_addr->sin_port = htons(port);
    server_addr->sin_family = AF_INET;
    server_addr->sin_addr.s_addr = INADDR_ANY;
}

void do_bind(int sock, struct sockaddr_in addr)
{
    if (bind(sock, (struct sockaddr *)&addr, sizeof(addr)) == -1)
        error("Bind error");
}

int do_accept(int sock, struct sockaddr * client_addr, socklen_t * client_size)
{
    int fd;
    fd = accept(sock, client_addr, client_size);
    if (fd == -1)
        error("Accept error");
    return fd;
}

char * read_line(int fd)
{
	int i = 0;
	char * d = malloc(64);
	char c;

	memset(d, 0, 64);

	while (1)
	{
		if ((read(fd, &c, 1) == 0)||(c == '\n'))
			break;

		d[i] = c;
		i++;
	}

	return d;
}

int count_lines(int fd)
{
	int count = 0;
	char * c = malloc(2);

	while (1)
	{
		if (read(fd, c+1, 1) == 0)
		{
			if (c[0] != '\n')
				count++;
			break;
		}

		if (c[1] == '\n')
			count++;
	}

	free(c);
	return count;
}

int main(int argc, char *argv[])
{

	if (argc < 3)
	{
		usage();
	}
	else
	{
		pid_t pid;
		int fd;
		int num_procs;
		int (*fd1)[2];
		int (*fd2)[2];
		int i;
		int struct_size = sizeof(struct sockaddr_in);

		int sock;
		struct sockaddr_in server_addr;
		struct sockaddr_in client_addr;
		char * arg_exec[3];
		char buf[LENGTH];
		char buffer[LENGTH];

		dsm_proc_t * machine;
		int * client_sock;
		 
		 /* Mise en place d'un traitant pour recuperer les fils zombies */      
		 /* XXX.sa_handler = sigchld_handler; */
		 
		/* lecture du fichier de machines */
		fd = open(argv[1], O_RDONLY);

		if (fd != -1)
		{
			/* 1- on recupere le nombre de processus a lancer */
			num_procs = count_lines(fd);
			
			/* 2- on recupere les noms des machines : le nom de */
			/* la machine est un des elements d'identification */
			machine = malloc(num_procs * sizeof(dsm_proc_t));
			lseek(fd, 0, SEEK_SET);

			for (i = 0; i < num_procs; ++i)
			{
				machine[i].rank = i;
				machine[i].name = read_line(fd);
			}

			close(fd);
		}
		else
		{
			exit(EXIT_FAILURE);
		}
		 
		/* creation de la socket d'ecoute */
		sock = do_socket(AF_INET, SOCK_STREAM, 0);
		init_serv_addr(&server_addr, 33000);
		do_bind(sock, server_addr);
		/* + ecoute effective */
		listen(sock, num_procs);

		/* Allocation de la mémoire pour la création des tubes */
		fd1 = malloc(2 * num_procs * sizeof(int));
		fd2 = malloc(2 * num_procs * sizeof(int));

		/* Allocation de la mémoire pour les sockets clients */
		client_sock = malloc(num_procs * sizeof(int));

		/* creation des fils */
		for(i = 0; i < num_procs ; i++)
		{
			/* creation du tube pour rediriger stdout */
			pipe(fd1[i]);

			/* creation du tube pour rediriger stderr */
			pipe(fd2[i]);

			pid = fork();

			if(pid == -1)
				ERROR_EXIT("fork");
			
			if (pid == 0)
			{ /* fils */

				/* redirection stdout */
				close(fd1[i][0]);
				close(STDOUT_FILENO);
				dup(fd1[i][1]);
				close(fd1[i][1]);

				/* redirection stderr */
				close(fd2[i][0]);
				close(STDERR_FILENO);
				dup(fd2[i][1]);
				close(fd2[i][1]);

				/* Récupération du PID du fils */
				machine[i].pid = getpid();

				execlp("echo", "echo", "coucou benJ!", NULL);

				/* Creation du tableau d'arguments pour le ssh */
				strcpy(arg_exec[0], "ssh");
				arg_exec[1] = machine[i].name;
				arg_exec[2] = NULL;

				/* jump to new prog : */
				execvp("ssh", arg_exec);

				/* Sortie de la boucle */
				break;
			}
			else if (pid > 0)
			{ /* pere */		      
				/* fermeture des extremites des tubes non utiles */
				close(fd1[i][1]);
				close(fd2[i][1]);

				num_procs_creat++;
			}
		}

		/* on accepte les connexions des processus dsm */
		//client_sock[i] = do_accept(sock, (struct sockaddr *)&client_addr, (socklen_t *)&struct_size);

		/* On recupere le nom de la machine distante */
		/* 1- d'abord la taille de la chaine */
		/* 2- puis la chaine elle-meme */

		/* On recupere le pid du processus distant */

		/* On recupere le numero de port de la socket */
		/* d'ecoute des processus distants */

		 
		/* envoi du nombre de processus aux processus dsm*/

		/* envoi des rangs aux processus dsm */

		/* envoi des infos de connexion aux processus */

		/* gestion des E/S : on recupere les caracteres */
		/* sur les tubes de redirection de stdout/stderr */
		if (pid > 0)
		{
			while(1)
			{
				for (i = 0; i < num_procs; i++)
				{
					/* Affichage du stdout */
					memset(buf, 0, LENGTH);
					if (read(fd1[i][0], buf, LENGTH) > 0)
					{
						memset(buffer, 0, LENGTH);
						sprintf(buffer, "[Proc %i : %s : stdout] %s\n", i, machine[i].name, buf);
						fflush(stdout);
						write(STDOUT_FILENO, buffer, LENGTH);
						fflush(stdout);
					}

					/* Affichage du stderr */
					memset(buf, 0, LENGTH);
					if (read(fd2[i][0], buf, LENGTH) > 0)
					{
						memset(buffer, 0, LENGTH);
						sprintf(buffer, "[Proc %i : %s : stderr] %s\n", i, machine[i].name, buf);
						fflush(stdout);
						write(STDOUT_FILENO, buffer, LENGTH);
						fflush(stdout);
					}
				}

				/* Pour l'instant */
				break;
			}
		 
			/* on attend les processus fils */
			for(i = 0; i < num_procs ; i++)
				wait(NULL);
		}

		/* on ferme les descripteurs proprement */

		/* on ferme la socket d'ecoute */
		close(sock);

		/* On libère toutes les mémoires allouées */
		free(client_sock);
		free(fd1);
		free(fd2);
		free(machine);
	}

	exit(EXIT_SUCCESS);  
}

