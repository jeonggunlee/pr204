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
		int num_lines;
		int num_procs = 0;
		int i;
		dsm_proc_t * machine;
		 
		 /* Mise en place d'un traitant pour recuperer les fils zombies */      
		 /* XXX.sa_handler = sigchld_handler; */
		 
		/* lecture du fichier de machines */
		fd = open(argv[1], O_RDONLY);

		if (fd != -1)
		{
			/* 1- on recupere le nombre de processus a lancer */
			num_lines = count_lines(fd);
			printf("%i\n", num_lines);
			
			/* 2- on recupere les noms des machines : le nom de */
			/* la machine est un des elements d'identification */
			machine = malloc(num_lines * sizeof(dsm_proc_t));
			lseek(fd, 0, SEEK_SET);

			for (i = 0; i < num_lines; ++i)
			{
				machine[i].name = read_line(fd);
			}

			close(fd);
		}
		else
		{
			exit(EXIT_FAILURE);
		}
		 
		 /* creation de la socket d'ecoute */
		 /* + ecoute effective */ 
		 
		 /* creation des fils */
		for(i = 0; i < num_procs ; i++)
		{
	
			/* creation du tube pour rediriger stdout */
			
			/* creation du tube pour rediriger stderr */
	
			pid = fork();

			if(pid == -1)
				ERROR_EXIT("fork");
			
			if (pid == 0)
			{ /* fils */	
				 
				 /* redirection stdout */	      
				 
				 /* redirection stderr */	      	      
				 
				 /* Creation du tableau d'arguments pour le ssh */ 
				 
				 /* jump to new prog : */
				 /* execvp("ssh",newargv); */

			}
			else if (pid > 0)
			{ /* pere */		      
				 /* fermeture des extremites des tubes non utiles */
				 num_procs_creat++;	      
			}
		}
		 
	 
		for(i = 0; i < num_procs ; i++)
		{
	
			/* on accepte les connexions des processus dsm */

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

		/* on ferme les descripteurs proprement */

		/* on ferme la socket d'ecoute */

		free(machine);
	}

	exit(EXIT_SUCCESS);  
}

