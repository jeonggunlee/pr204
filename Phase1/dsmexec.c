#include "common_impl.h"

/* variables globales */

/* un tableau gerant les infos d'identification */
/* des processus dsm */
dsm_proc_t *proc_array = NULL; 

/* le nombre de processus effectivement crees */
volatile int num_procs_creat = 0;

void error(const char* msg) 
{
	perror(msg);
	exit(-1);
}

void usage(void)
{
	fprintf(stdout,"Usage : dsmexec machine_file executable arg1 arg2 ...\n");
	fflush(stdout);
	exit(EXIT_FAILURE);
}

void sigchld_handler(int sig)
{
	 /* on traite les fils qui se terminent */
	 /* pour eviter les zombies */
}

char * read_line(int fd)
{
	int i = 0;
	char * d = malloc(64);
	char c;

	memset(d, 0, 64);

	while (1)
	{
		if ((read(fd, &c, 1) == 0) || (c == '\n'))
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

void init_serv_addr(struct sockaddr_in *serv_addr, int port) 
{
	memset(serv_addr, 0, sizeof(*serv_addr));
    serv_addr->sin_family = AF_INET; 
    serv_addr->sin_addr.s_addr = INADDR_ANY;
    serv_addr->sin_port = htons(port);
}

int do_socket(int domain, int type, int protocol) 
{
	int fd;
	int yes = 1;

	fd = socket(domain, type, protocol);

	if (fd == -1)
		error("ERROR : cannot create socket");
	if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1)
		error("ERROR setting socket options");
	
	return fd;
}

int do_bind(int fd, struct sockaddr_in addr)
{
    if (bind(fd, (struct sockaddr*)&addr, sizeof(addr)) == -1)
    {
        error("ERROR : cannot bind");
        exit(EXIT_FAILURE);
    }
    
    return 0;
}

int do_accept(int fd, struct sockaddr *addr, socklen_t *addrlen)
{
    int res = accept(fd, addr, addrlen);
    if (res == -1)
        error("ERROR : accept failed");

    return res;
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
		int struct_size = sizeof(struct sockaddr_in);
		int status;
		int fd;
		int num_procs = 0;
		int i;
		int sock;
		int fd1[2];
		int fd2[2];

		int *client_sock;

		dsm_proc_t * machine;
		struct sockaddr_in server_addr, client_addr;

		char *arg_exec[3];

		/* Mise en place d'un traitant pour recuperer les fils zombies */      
		/* XXX.sa_handler = sigchld_handler; */
		 
		/* lecture du fichier de machines */
		fd = open(argv[1], O_RDONLY);

		if (fd != -1)
		{
			/* 1- on recupere le nombre de processus a lancer */
			num_procs = count_lines(fd);
			// printf("%i\n", num_lines);
			
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
		
		//Prepare the sockaddr_in structure to store server info
    	init_serv_addr(&server_addr, 33000);

	    //Assign address and port to socket using bind()
	    do_bind(sock, server_addr);

		/* + ecoute effective */    
	    listen(sock, num_procs);
		
		/*!!!!!!DOUTE ICI!!!!!!!!*/
		/* creation du tube pour rediriger stdout */
		pipe(fd1);			
		/* creation du tube pour rediriger stderr */
		pipe(fd2);

		/* creation des fils */
		for(i = 0; i < num_procs ; i++)
		{
			pid = fork();

			if(pid == -1)
				ERROR_EXIT("fork");
			
			if (pid == 0)
			{ /* fils */	
				
				/*Récupération du pid du fils*/
				machine[i].pid = getpid();

				/* fermeture des extremites inutiles */
				// close(fd1[0]);
				// close(fd2[0]);

				/* redirection stdout */
				close(STDOUT_FILENO);
				dup(fd1[1]);
				close(fd1[1]);

				/* redirection stderr */
				close(STDERR_FILENO);
				dup(fd2[1]);
				close(fd2[1]);

				execlp("echo", "echo", "test", NULL);
				
				/* Creation du tableau d'arguments pour le ssh */
				strcpy(arg_exec[0], "ssh");
				arg_exec[1] = machine[i].name;
				arg_exec[2] = NULL;
				 
				/* jump to new prog : */
				execvp("ssh", arg_exec);
			}
			else if (pid > 0)
			{ /* pere */		      
				/* fermeture des extremites des tubes non utiles */
				close(fd1[1]);
				close(fd2[1]);

				num_procs_creat++;	      
			}
		}
		
		client_sock = malloc(num_procs * sizeof(int));
	 
		for(i = 0; i < num_procs ; i++)
		{
	
			/* on accepte les connexions des processus dsm */
			// client_sock[i] = do_accept(sock, (struct sockaddr*)&client_addr, (socklen_t*)&struct_size);


			/*  On recupere le nom de la machine distante */
			/* 1- d'abord la taille de la chaine */
			/* 2- puis la chaine elle-meme */

			/* On recupere le pid du processus distant  */

			/* On recupere le numero de port de la socket */
			/* d'ecoute des processus distants */

		}
		 
		/* envoi du nombre de processus aux processus dsm*/

		/* envoi des rangs aux processus dsm */

		/* envoi des infos de connexion aux processus */

		/* gestion des E/S : on recupere les caracteres */
		/* sur les tubes de redirection de stdout/stderr */     
		/* while(1)
		{
			je recupere les infos sur les tubes de redirection
			jusqu'à ce qu'ils soient inactifs (ie fermes par les
			processus dsm ecrivains de l'autre cote ...)

		};
		*/
		 
		/* on attend les processus fils */
		for (i = 0; i < num_procs; i++)
			waitpid(pid, &status, WEXITED);

		/* on ferme les descripteurs proprement */


		/* on ferme la socket d'ecoute */
		close(sock);

		free(machine);
	}

	exit(EXIT_SUCCESS);  
}

