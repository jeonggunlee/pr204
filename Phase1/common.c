#include "common_impl.h"

int count_digits(int number)
{
	if (number != 0)
		return (int)(1+log10(abs(number)));
	return 1;
}

char * string_copy(char * src)
{
	int n = strlen(src);
	char * dst = malloc(n+1);

	memset(dst, '\0', n+1);
	
	strncpy(dst, src, n);

	return dst;
}

char * int_copy(int number)
{
	int n = count_digits(number);
	char * dst = malloc(n+1);

	memset(dst, '\0', n+1);
	
	sprintf(dst, "%d", number);

	return dst;
}

void get_addr_info(struct sockaddr_in * server, char * name, int port)
{
	struct hostent * server_host;
	memset(server, 0, sizeof(struct sockaddr_in));
	server->sin_family = AF_INET;
	server->sin_port = htons(port);
	server_host = gethostbyname(name);
	memcpy(&(server->sin_addr), server_host->h_addr, server_host->h_length);
}

void do_connect(int sock, struct sockaddr_in addr)
{
	if (connect(sock, (struct sockaddr *)&addr, sizeof(struct sockaddr_in)) == -1)
		ERROR_EXIT("Connect error");
}

int init_socket(int type)
{
	int fd = -1;
	int yes = 1;

	// creation de la socket
	fd = socket(AF_INET, type, 0);

	// verification de la validite de la socket
	if (fd == -1)
		ERROR_EXIT("Socket error");

	// socket option pour le probleme "already in use"
	if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1)
		ERROR_EXIT("Error setting socket options");

	return fd;
}

int creer_socket(int type, int * port_num)
{
	int fd = init_socket(type);
	int size_addr_len = sizeof(struct sockaddr_in);
	struct sockaddr_in server_addr;

	// initialisation du serveur
	memset(&server_addr, 0, sizeof(struct sockaddr_in));
	server_addr.sin_port = htons(0);
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = INADDR_ANY;

	// attachement de la nouvelle socket
	if (bind(fd, (struct sockaddr *)&server_addr, sizeof(struct sockaddr_in)) == -1)
		ERROR_EXIT("Bind error")

	getsockname(fd, (struct sockaddr *)&server_addr, (socklen_t *)&size_addr_len);

	*port_num = ntohs(server_addr.sin_port);
	
	return fd;
}

int do_accept(int sock, struct sockaddr * client_addr, socklen_t * client_size)
{
	int fd;

	fd = accept(sock, client_addr, client_size);

	if (fd == -1)
		ERROR_EXIT("Accept error");

	return fd;
}

void handle_client_message(int fd, char * msg)
{
	send(fd, msg, strlen(msg), 0);
}

void receive(int fd, char * buf)
{
	int i = 0;

	memset(buf, '\0', LENGTH);
	
	while (recv(fd, buf+i, 1, 0) > 0)
	{
		if (buf[i] == '\n')
		{
			buf[i] = '\0';
			break;
		}

		i++;

		if (i == LENGTH-1)
			break;
	}
}

void * arguments(int fd, char * type, dsm_proc_t * proc)
{
	proc_args_t * args = malloc(sizeof(proc_args_t));
	args->fd = fd;
	args->type = string_copy(type);
	args->machine = string_copy(proc->connect_info.hostname);
	args->rank = proc->connect_info.rank;
	return args;
}