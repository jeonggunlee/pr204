#include <semaphore.h>
#include "common_impl.h"

// variables globales

// un tableau gerant les infos d'identification
// des processus dsm
dsm_proc_t * proc_array = NULL;

// le nombre de processus effectivement crees
volatile int num_procs_creat = 0;

void usage(void)
{
	fprintf(stdout,"Usage : dsmexec machine_file executable arg1 arg2 ...\n");
	fflush(stdout);
	exit(EXIT_FAILURE);
}

void sigchld_handler(int sig)
{
	pid_t pid;

	while ((pid = waitpid(-1, NULL, WNOHANG)) != -1)
	{
		if (pid > 0)
			num_procs_creat--;
		sleep(1);
	}
}

ssize_t count_lines(const char * filename, int * num_procs)
{
	int fd;
	ssize_t size = 0;
	char c[2];

	memset(c, 0, 2);

	fd = open(filename, O_RDONLY);

	if (fd != -1)
	{
		while (read(fd, c+1, 1) > 0)
		{
			if (c[1] == '\n')
				(*num_procs)++;
			size++;
		}

		if (c[1] != '\n')
			(*num_procs)++;
	}
	else
	{
		fprintf(stdout, "Fichier invalide\n");
		fflush(stdout);
		exit(EXIT_FAILURE);
	}

	close(fd);
	return size;
}

void machine_names(const char * filename, char ** machines, ssize_t size)
{
	int i;
	int k = 0;
	int cursor = 0;
	int fd;
	char buf[size+1];

	fd = open(filename, O_RDONLY);

	if (fd != -1)
	{
		read(fd, buf, size);

		for (i = 0; i <= size; i++)
		{
			if ((buf[i] == '\n') || ((i == size) && (buf[size] != '\n')))
			{
				machines[k] = malloc(i-cursor+1);
				memset(machines[k], '\0', i-cursor+1);
				strncpy(machines[k], buf+cursor, i-cursor);
				cursor = i+1;
				k++;
			}
		}
	}
	else
	{
		fprintf(stdout, "Fichier invalide\n");
		fflush(stdout);
		exit(EXIT_FAILURE);
	}

	close(fd);
}

void * display(void * arguments)
{
	int n;
	int offset;
	int fd;
	int rank;
	char * type;
	char * machine;
	char buf[LENGTH];
	char buffer[LENGTH];

	proc_args_t * args = arguments;
	fd = args->fd;
	type = args->type;
	machine = args->machine;
	rank = args->rank;

	offset = 15 + count_digits(rank) + strlen(machine) + strlen(type);

	while(1)
	{
		memset(buf, '\0', LENGTH);
		memset(buffer, '\0', LENGTH);
		
		n = read(fd, buf, LENGTH - offset);

		if (n > 0)
		{
			sprintf(buffer, "[Proc %d : %s : %s] %s\n", rank, machine, type, buf);
			write(STDOUT_FILENO, buffer, strlen(buffer));
			fflush(stdout);
		}
		else
			break;
	}

	free(type);
	free(args);
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
		int num_procs = 0;
		int i, j;
		int (*fd1)[2];
		int (*fd2)[2];
		int port;
		int * client_fd;
		ssize_t size;

		char ** machines;
		char ** newargv;
		char hostname[LENGTH];
		char buf[LENGTH];
		const char * machine_file = argv[1];

		struct sigaction zombie;
		pthread_t * thr1;
		pthread_t * thr2;

		struct sockaddr_in client_addr;
		socklen_t client_size = sizeof(struct sockaddr_in);
		
		// Mise en place d'un traitant pour recuperer les fils zombies
		memset(&zombie, 0, sizeof(struct sigaction));
		zombie.sa_handler = sigchld_handler;
		zombie.sa_flags = SA_RESTART;
		sigaction(SIGCHLD, &zombie, NULL);
		
		// Lecture du fichier de machines
		// 1- on recupere le nombre de processus a lancer
		size = count_lines(machine_file , &num_procs);

		// 2- on recupere les noms des machines : le nom de la machine est un des elements d'identification
		machines = malloc(num_procs * sizeof(char *));
		machine_names(machine_file, machines, size);

		// creation de la socket d'ecoute
		fd = creer_socket(SOCK_STREAM, &port);
		
		// ecoute effective
		listen(fd, num_procs);

		memset(hostname, '\0', LENGTH);
		gethostname(hostname, LENGTH);

		// allocation des memoires
		fd1 = malloc(2 * num_procs * sizeof(int));
		fd2 = malloc(2 * num_procs * sizeof(int));
		newargv = malloc((argc+3) * sizeof(char *));
		client_fd = malloc(num_procs * sizeof(int));
		proc_array = malloc(num_procs * sizeof(dsm_proc_t));
		
		// creation des fils
		for (i = 0; i < num_procs ; i++)
		{
			// creation du tube pour rediriger stdout
			pipe(fd1[i]);

			// creation du tube pour rediriger stderr
			pipe(fd2[i]);
			
			pid = fork();

			if (pid == -1) ERROR_EXIT("fork");
			
			if (pid == 0)
			{ // fils
				
				// redirection stdout
				close(fd1[i][0]);
				close(STDOUT_FILENO);
				dup(fd1[i][1]);
				close(fd1[i][1]);

				// redirection stderr
				close(fd2[i][0]);
				close(STDERR_FILENO);
				dup(fd2[i][1]);
				close(fd2[i][1]);

				// fermeture des extremites des autres tubes
				for (j = 0; j < i; j++)
				{
					close(fd1[j][0]);
					close(fd2[j][0]);
				}

				// creation du tableau d'arguments pour le ssh
				// 1- ssh
				newargv[0] = string_copy("ssh");

				// 2- nom de la machine distante
				newargv[1] = string_copy(machines[i]);

				// 3- dsmwrap
				newargv[2] = string_copy(DSMWRAP_PATH);

				// 4- executable + ses arguments
				for (j = 2; j < argc; j++)
					newargv[j+1] = string_copy(argv[j]);

				// 5- port de dsmexec
				newargv[argc+1] = int_copy(port);

				// 6- ip courante
				newargv[argc+2] = string_copy(hostname);

				// 7- NULL
				newargv[argc+3] = NULL;

				// jump to new prog :
				execvp(newargv[0], newargv);
			}
			else if (pid > 0)
			{ // pere
				// fermeture des extremites des tubes non utiles
				close(fd1[i][1]);
				close(fd2[i][1]);

				num_procs_creat++;
			}
		}

		for (i = 0; i < num_procs; i++)
		{	
			// on accepte les connexions des processus dsm
			client_fd[i] = do_accept(fd, (struct sockaddr *)&client_addr, &client_size);

			// On recupere le nom de la machine distante
			receive(client_fd[i], buf);
			proc_array[i].connect_info.hostname = string_copy(buf);
			
			// On recupere le pid du processus distant
			receive(client_fd[i], buf);
			proc_array[i].pid = atoi(string_copy(buf));

			// On recupere le numero de port de la socket
			// d'ecoute des processus distants
			receive(client_fd[i], buf);
			proc_array[i].connect_info.port = atoi(string_copy(buf));

			proc_array[i].connect_info.rank = i;
		}
		
		// envoi du nombre de processus aux processus dsm
		
		// envoi des rangs aux processus dsm
		
		// envoi des infos de connexion aux processus
		
		// gestion des E/S : on recupere les caracteres
		// sur les tubes de redirection de stdout/stderr
		thr1 = malloc(num_procs * sizeof(pthread_t));
		thr2 = malloc(num_procs * sizeof(pthread_t));

		for (i = 0; i < num_procs; i++)
		{
			// je recupere les infos sur les tubes de redirection
			// jusqu'à ce qu'ils soient inactifs (ie fermes par les
			// processus dsm ecrivains de l'autre cote ...)
			pthread_create(thr1+i, NULL, display, arguments(fd1[i][0], "stdout", proc_array+i));
			pthread_create(thr2+i, NULL, display, arguments(fd2[i][0], "stderr", proc_array+i));
		}

		for (i = 0; i < num_procs; i++)
		{
			pthread_join(thr1[i], NULL);
			pthread_join(thr2[i], NULL);
		}
		
		// on attend les processus fils
		while(wait(NULL) > 0);
		
		// on ferme les descripteurs proprement
		
		// on ferme la socket d'ecoute
		close(fd);

		// on libere les memoires allouees
		free(proc_array);
		free(thr1);
		free(thr2);
		for (i = 0; i < num_procs; i++)
			free(machines[i]);
		for (i = 0; i < argc+3; i++)
			free(newargv[i]);
		free(client_fd);
		free(newargv);
		free(machines);
		free(fd1);
		free(fd2);
	}
	exit(EXIT_SUCCESS);
}
