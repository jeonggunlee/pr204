#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "common_impl.h"

#define FD_DSMEXEC  3
#define FD_LISTEN	4

int main(int argc, char *argv[])
{
	int fd;
	int i;
	char str[LENGTH];
	char exec_path[LENGTH];
	char * wd_ptr = NULL;

	wd_ptr = getcwd(str, LENGTH);
	fprintf(stdout, "Working dir is %s\n", str);

	fprintf(stdout, "Number of args : %i\n", argc);
	for(i= 0; i < argc ; i++)
	  fprintf(stderr, "arg[%i] : %s\n", i, argv[i]);
	 
	sprintf(exec_path, "%s/%s", str, "titi");	      
	fd = open(exec_path, O_RDONLY);
	if(fd == -1) perror("open");
	fprintf(stdout,"================ Valeur du descripteur : %i\n",fd);

	close(fd);

	fflush(stdout);
	fflush(stderr);
	return 0;
}

/*
#include <stdio.h>

int main(int argc, char ** argv)
{
	int i;
	int dsm_node_id;
	int num_procs;
	int fd = FD_DSMEXEC;
	char buf[LENGTH];
	dsm_proc_t * proc_array;

	receive(fd, buf);
	num_procs = atoi(buf);

	proc_array = malloc((num_procs-1) * sizeof(dsm_proc_t));

	receive(fd, buf);
	dsm_node_id = atoi(buf);

	for (i = 0; i < num_procs-1; i++)
	{
		receive(fd, buf);
		proc_array[i].connect_info.hostname = string_copy(buf);

		receive(fd, buf);
		proc_array[i].connect_info.port = atoi(buf);
	}

	free(proc_array);

	return 0;
}*/