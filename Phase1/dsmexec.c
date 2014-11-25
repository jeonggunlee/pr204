#include "common_impl.h"

/* variables globales */

/* un tableau gerant les infos d'identification */
/* des processus dsm */
dsm_proc_t * proc_array = NULL; 

/* le nombre de processus effectivement crees */
volatile int num_procs_creat = 0;

void usage(void)
{
	fprintf(stdout,"Usage : dsmexec machine_file executable arg1 arg2 ...\n");
	fflush(stdout);
	exit(EXIT_FAILURE);
}

void sigchld_handler(int sig)
{
	while(num_procs_creat)
		usleep(500);
}

int do_accept(int sock, struct sockaddr * client_addr, socklen_t * client_size)
{
    int fd;
    fd = accept(sock, client_addr, client_size);
    if (fd == -1)
        ERROR_EXIT("Accept error");
    return fd;
}

ssize_t read_fd(int fd, char * buf)
{
	int i = 0;
	char c;

	memset(buf, 0, LENGTH);

	while (1)
	{
		if ((read(fd, &c, 1) == 0)||(c == '\n')||(i == LENGTH))
			break;

		buf[i] = c;
		i++;
	}

	return i;
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

void * proc_display(void * arguments)
{
	int n;
	short toto;
	int fd;
	const char * type;
	char buf[LENGTH];
	char buffer[LENGTH];

	proc_args_t * args = arguments;

	fd = args->fd;
	type = args->type;

	while(++toto && toto < 100)
	{
		memset(buf, 0, LENGTH);

		n = read(fd, buf, LENGTH);

		if (n > 0)
		{
			memset(buffer, 0, LENGTH);
			sprintf(buffer, "[%s] %s\n", type, buf);
			fflush(stdout);
			write(STDOUT_FILENO, buffer, LENGTH);
			fflush(stdout);
		}
	}

	pthread_exit(NULL);
}

int main(int argc, char ** argv)
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
		int port;
		int (*fd1)[2];
		int (*fd2)[2];
		int i, j;
		int struct_size = sizeof(struct sockaddr_in);
		struct sigaction p_action;

		int sock;
		struct sockaddr_in client_addr;

		dsm_proc_t * machine;
		int * client_sock;
		pthread_t * thread;
		
		/* Mise en place d'un traitant pour recuperer les fils zombies */      
		memset(&p_action, 0, sizeof(struct sigaction));
		p_action.sa_handler = sigchld_handler;

		sigaction(SIGCHLD, &p_action, NULL);
		 
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
				read_fd(fd, machine[i].name);
			}
		}
		else
		{
			exit(EXIT_FAILURE);
		}
		 
		/* creation de la socket d'ecoute */
		port = 1024;
		sock = creer_socket(SOCK_STREAM, &port);

		/* + ecoute effective */
		listen(sock, num_procs);

		/* Allocation de la mémoire pour la création des tubes */
		fd1 = malloc(2 * num_procs * sizeof(int));
		fd2 = malloc(2 * num_procs * sizeof(int));

		/* Allocation de la mémoire pour le tableau de socks d'initialisation */
		client_sock = malloc(num_procs * sizeof(int));

		/* Allocation de la mémoire pour le tableau d'arguments */
		char * arg_exec[argc + num_procs + 5];

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

				for (j = 0; j < i; j++)
				{
					close(fd1[j][0]);
					close(fd1[j][0]);
				}

				/* Recuperation du PID du fils */
				machine[i].pid = getpid();

				/* Creation du tableau d'arguments pour le ssh */
				arg_exec[0] = "ssh";
				arg_exec[1] = machine[i].name;
				arg_exec[2] = "~/PR204/Phase1/bin/dsmwrap";

				/* Introduction des arguments */
				for (j = 3; j < argc+1; j++)
				{
					arg_exec[j] = argv[j-1];
				}

				/* Introduction des arguments utiles mais non lances sur ssh */
				/* 1 - le nom des machines */
				for (j = argc+1; j < argc+1+num_procs; j++)
					arg_exec[j] = machine[j-argc-1].name;

				/* 2 - port */
				arg_exec[argc+num_procs+1] = malloc(5);
				memset(arg_exec[argc+num_procs+1], 0, 5);
				sprintf(arg_exec[argc+num_procs+1], "%d", port);
				
				/* 3 - le nom de la machine courante */
				arg_exec[argc+num_procs+2] = "localhost";

				/* 4 - transmission de num_procs */
				arg_exec[argc+num_procs+3] = malloc((int)(1+log10(num_procs)));
				memset(arg_exec[argc+num_procs+3], 0, (int)(1+log10(num_procs)));
				sprintf(arg_exec[argc+num_procs+3], "%d", num_procs);
				arg_exec[argc+num_procs+4] = NULL;

				/* jump to new prog : */
				execvp(arg_exec[0], arg_exec);
			}
			else if (pid > 0)
			{ /* pere */		      
				/* fermeture des extremites des tubes non utiles */
				close(fd1[i][1]);
				close(fd2[i][1]);

				num_procs_creat++;
			}
		}

		for (i = 0; i < num_procs; i++)
		{
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
		}

		/* gestion des E/S : on recupere les caracteres */
		/* sur les tubes de redirection de stdout/stderr */
		thread = malloc(2 * num_procs * sizeof(pthread_t));

		for (i = 0; i < num_procs; i++)
		{
			pthread_create(thread+i, NULL, proc_display, arguments(fd1[i][0], "stdout"));
			pthread_create(thread+i+num_procs, NULL, proc_display, arguments(fd2[i][0], "stderr"));
		}
	 
		/* on attend les processus fils */
		for(i = 0; i < num_procs ; i++)
		{
			pthread_join(thread[i], NULL);
			pthread_join(thread[i+num_procs], NULL);
			wait(NULL);
		}

		/* on ferme les descripteurs proprement */
		close(fd);

		/* on ferme la socket d'ecoute */
		close(sock);

		/* On libère toutes les mémoires allouées */
		free(thread);
		free(client_sock);
		free(fd1);
		free(fd2);
		free(machine);
	}

	exit(EXIT_SUCCESS);  
}

