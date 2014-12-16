#include "dsm.h"
#include "common_impl.h"

int DSM_NODE_NUM; // nombre de processus dsm
int DSM_NODE_ID;  // rang (= numero) du processus
int * fd_connect = NULL; // sockets
int * fd_client = NULL; // sockets
dsm_proc_t * proc_array = NULL;

// indique l'adresse de debut de la page de numero numpage
static char * num2address(int numpage)
{ 
	char * pointer = (char *)(BASE_ADDR+(numpage*(PAGE_SIZE)));

	if (pointer >= (char *)TOP_ADDR)
	{
		fprintf(stderr,"[%i] Invalid address !\n", DSM_NODE_ID);
		return NULL;
	}
	else
	{
		return pointer;
	}
}

// fonctions pouvant etre utiles
static void dsm_change_info(int numpage, dsm_page_state_t state, dsm_page_owner_t owner)
{
	if ((numpage >= 0) && (numpage < PAGE_NUMBER)) 
	{	
		if (state != NO_CHANGE )
			table_page[numpage].status = state;
		if (owner >= 0 )
			table_page[numpage].owner = owner;
	}
	else 
	{
		fprintf(stderr,"[%i] Invalid page number !\n", DSM_NODE_ID);
	}
}

static dsm_page_owner_t get_owner(int numpage)
{
	return table_page[numpage].owner;
}

static dsm_page_state_t get_status(int numpage)
{
	return table_page[numpage].status;
}

// Allocation d'une nouvelle page
static void dsm_alloc_page(int numpage)
{
	char * page_addr = num2address(numpage);
	mmap(page_addr, PAGE_SIZE, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
}

// Changement de la protection d'une page
static void dsm_protect_page(int numpage, int prot)
{
	char * page_addr = num2address(numpage);
	mprotect(page_addr, PAGE_SIZE, prot);
}

static void dsm_free_page(int numpage)
{
	char * page_addr = num2address(numpage);
	munmap(page_addr, PAGE_SIZE);
}

static void * dsm_comm_daemon(void * arg)
{  
	while(1)
	{
	   // a modifier
		printf("[%i] Waiting for incoming reqs \n", DSM_NODE_ID);
		sleep(2);
	}

	pthread_exit(NULL);
}

static void dsm_handler()
{
	// A modifier
	printf("[%i] FAULTY  ACCESS !!! \n", DSM_NODE_ID);
	abort();
}

// traitant de signal adequat
static void segv_handler(int sig, siginfo_t * info, void * context)
{
	// A completer
	// adresse qui a provoque une erreur
	void * addr = info->si_addr;

	// Si ceci ne fonctionne pas, utiliser a la place :
	/*
	#ifdef __x86_64__
	void *addr = (void *)(context->uc_mcontext.gregs[REG_CR2]);
	#elif __i386__
	void *addr = (void *)(context->uc_mcontext.cr2);
	#else
	void  addr = info->si_addr;
	#endif
	*/
	/*pour plus tard (question ++):
	dsm_access_t access  = (((ucontext_t *)context)->uc_mcontext.gregs[REG_ERR] & 2) ? WRITE_ACCESS : READ_ACCESS;   
	*/ 
	// adresse de la page dont fait partie l'adresse qui a provoque la faute
	void * page_addr  = (void *)(((unsigned long) addr) & ~(PAGE_SIZE-1));

	if ((addr >= (void *)BASE_ADDR) && (addr < (void *)TOP_ADDR))
	{
		dsm_handler();
	}
	else
	{
		// SIGSEGV normal : ne rien faire
	}
}

void * do_connect_thread(void * args)
{
	int i;

	struct sockaddr_in * sockaddr = malloc((DSM_NODE_NUM-1) * sizeof(struct sockaddr_in));
	// pour s'assurer que connect soit effectue apres accept
	sleep(1);

	for (i = 0; i < DSM_NODE_NUM-1; i++)
	{
		get_addr_info(sockaddr+i, proc_array[i].connect_info.hostname, proc_array[i].connect_info.port);
		fd_connect[i] = init_socket(SOCK_STREAM);
		do_connect(fd_connect[i], sockaddr[i]);
	}

	free(sockaddr);

	pthread_exit(NULL);
}

void * do_accept_thread(void * args)
{
	int i;
	struct sockaddr_in * client_addr;
	socklen_t client_size = sizeof(struct sockaddr_in);

	client_addr = malloc((DSM_NODE_NUM-1) * sizeof(struct sockaddr_in));		// FREE !!!

	for (i = 0; i < DSM_NODE_NUM-1; i++)
	{
		fd_client[i] = do_accept(FD_LISTEN, (struct sockaddr *)client_addr+i, &client_size);
	}

	free(client_addr);
	pthread_exit(NULL);
}

// Seules ces deux dernieres fonctions sont visibles et utilisables
// dans les programmes utilisateurs de la DSM                      
char * dsm_init(int argc, char ** argv)
{   
	struct sigaction act;
	int i;
	int index;
	char buf[LENGTH];

	pthread_t thread_connect;
	pthread_t thread_accept;

	// reception du nombre de processus dsm envoye
	// par le lanceur de programmes (DSM_NODE_NUM)
	receive(FD_DSMEXEC, buf);
	DSM_NODE_NUM = atoi(buf);
	
	// reception de mon numero de processus dsm envoye
	// par le lanceur de programmes (DSM_NODE_ID)
	receive(FD_DSMEXEC, buf);
	DSM_NODE_ID = atoi(buf);

	// allocation de la memoire pour les autres dsm
	fd_connect = malloc((DSM_NODE_NUM-1) * sizeof(int));
	fd_client = malloc((DSM_NODE_NUM-1) * sizeof(int));
	proc_array = malloc((DSM_NODE_NUM-1) * sizeof(dsm_proc_t));

	// reception des informations de connexion des autres
	// processus envoyees par le lanceur
	for (i = 0; i < DSM_NODE_NUM-1; i++)
	{
		// nom des autres machines
		receive(FD_DSMEXEC, buf);
		proc_array[i].connect_info.hostname = string_copy(buf);

		// numero de port
		receive(FD_DSMEXEC, buf);
		proc_array[i].connect_info.port = atoi(buf);
	}

	listen(FD_LISTEN, DSM_NODE_NUM-1);

	// initialisation des connexions avec les autres processus
	// pas besoin de mutex
	// connect
	pthread_create(&thread_connect, NULL, do_connect_thread, NULL);

	// accept
	pthread_create(&thread_accept, NULL, do_accept_thread, NULL);

	pthread_join(thread_connect, NULL);
	pthread_join(thread_accept, NULL);

	// Allocation des pages en tourniquet
	for(index = 0; index < PAGE_NUMBER; index++)
	{	
		if ((index % DSM_NODE_NUM) == DSM_NODE_ID)
			dsm_alloc_page(index);	     

		dsm_change_info( index, WRITE, index % DSM_NODE_NUM);
	}
   
	// mise en place du traitant de SIGSEGV
	act.sa_flags = SA_SIGINFO;
	act.sa_sigaction = segv_handler;
	sigaction(SIGSEGV, &act, NULL);
   
	// creation du thread de communication
	// ce thread va attendre et traiter les requetes
	// des autres processus
	pthread_create(&comm_daemon, NULL, dsm_comm_daemon, NULL);
   
	// Adresse de début de la zone de mémoire partagée
	return ((char *)BASE_ADDR);
}

void dsm_finalize() 
{
	int i;

	// fermer proprement les connexions avec les autres processus
	for (i = 0; i < DSM_NODE_NUM; i++)
	{
		close(fd_connect[i]);
		close(fd_client[i]);
	}

	// terminer correctement le thread de communication
	// pour le moment, on peut faire :
	pthread_cancel(comm_daemon);
	free(proc_array);
	free(fd_client);
	free(fd_connect);
   
	return;
}

