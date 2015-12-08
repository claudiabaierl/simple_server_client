/**
 * @file simple_message_server.c
 *
 * VCS TCP/IP Client
 *
 * @author: Claudia Baierl - ic14b003 <ic14b003@technikum-wien.at>
 * @author: Zübide Sayici - ic14b002 <ic14b002@technikum-wien.at>
 *
 * @version $Revision: xxx $
 *
 * Last Modified: $Author: xxxxx $
 */

/*
 * ----------------------------- includes -------------------------
 */

#include <arpa/inet.h>
#include <errno.h>
#include <limits.h>
#include <netdb.h>
#include <netinet/in.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <limits.h>
#include "simple_message_client_commandline_handling.h"
#include <sys/wait.h>
#include <signal.h>
#include <getopt.h>


/*
 * ---------------------------------- defines ------------------------
 */
/* Backlog */
#define LISTEN 24
/* path to servers business logic */
#define PATHSERVERLOGIC "/usr/local/bin/simple_message_server_logic"
#define PORT_MIN 0
#define PORT_MAX 65535
#define STRTOL_BASE 10

/*
 * ---------------------------------- globals ------------------------
 */
const char *prg_name;


/*
 * ---------------------------------- function prototypes ------------
 */

void my_usage(FILE * out, int exit_status);
void check_parameters_server(int argc, char *argv[], const char **port);
void signal_child(int sig);


/**
 *
 * \brief Main function implements a server which connects to a client
 * Main entry point
 *
 * \param argc passes the number of arguments
 * \param argv passes the arguments (programme name is argv[0]
 *
 * \return EXIT_SUCCESS when no error occurred
 * \return EXIT_FAILURE when an error occurred
 *
 */

int main(int argc, char *argv[])
{

	struct addrinfo hints;
	struct addrinfo *server, *rp;
	int socket_desc, new_socket_desc;
	int check;
	int child;
	struct sockaddr_storage address;
	socklen_t address_length;
	/* server port number */
	const char *port = NULL;
	int y = 0;

	prg_name = argv[0];
	check_parameters_server(argc, argv, &port);

	memset(&hints, 0, sizeof(hints));
	/* server connects to IPv4 address */
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;

	y = 1;

	check = getaddrinfo(NULL, port, &hints, &server);
	if(check != 0)
	{
		fprintf(stderr, "%s: error getaddrinfo: %s\n", prg_name, gai_strerror(check));
		return EXIT_FAILURE;
	}

	for (rp = server; rp != NULL; rp = rp->ai_next)
	{
		socket_desc = socket(server->ai_family, server->ai_socktype, server->ai_protocol);
		if(socket_desc == -1)
		{
			fprintf(stderr, "%s: error socket: %s\n", prg_name, strerror(errno));
			freeaddrinfo(server);
			return EXIT_FAILURE;
		}

		if(setsockopt(socket_desc, SOL_SOCKET, SO_REUSEADDR, &y, sizeof(int)) == -1)
		{
			fprintf(stderr, "%s: error setsockopt %s\n", prg_name, strerror(errno));
			close(socket_desc);
			freeaddrinfo(server);
			return EXIT_FAILURE;
		}
		/* if bind is not successful, try with the next address */
		if(bind(socket_desc, server->ai_addr, server->ai_addrlen) == -1)
		{
			fprintf(stderr, "%s: error bind %s\n", prg_name, strerror(errno));
			close(socket_desc);
			freeaddrinfo(server);
			continue;
		}
		break;
	}

	if(rp == NULL)
	{
		fprintf(stderr, "%s: server binding failed\n", prg_name);
		return EXIT_FAILURE;
	}

	freeaddrinfo(server);

	/* listen on socket */
	if(listen(socket_desc, LISTEN) == -1)
	{
		fprintf(stderr, "%s: error because of too many connections %s\n", prg_name, strerror(errno));
		close(socket_desc);
		return EXIT_FAILURE;
	}

	signal(SIGCHLD, signal_child);

	/* loop until accept was successful */
	for(;;)
	{
		address_length = sizeof(address);
		new_socket_desc = accept(socket_desc, (struct sockaddr *) &address, &address_length);

		/* error handling for accept */
		if(new_socket_desc == -1)
		{
			/* if the system call was interrupted by a signal that was caught before a valid connection was
			 * established, continue */
			if(errno == EINTR)
			{
				continue;
			}
			else
			{
				fprintf(stderr, "%s: error accept %s\n", prg_name, strerror(errno));
				close(socket_desc);
				return EXIT_FAILURE;
			}
		}

		/* fork child process for execution of business logic */
		child = fork();

		if(child == -1)
		{
			close(new_socket_desc);
			close(socket_desc);
			fprintf(stderr, "%s: fork error %s\n", prg_name, strerror(errno));
			return EXIT_FAILURE;
		}
		else if(child == 0)
		{
			close(socket_desc);
			if(dup2(new_socket_desc, 0) == -1)
			{
				close(new_socket_desc);
				return EXIT_FAILURE;
			}
			if(dup2(new_socket_desc, 1) == -1)
			{
				close(new_socket_desc);
				return EXIT_FAILURE;
			}
			if(execlp(PATHSERVERLOGIC, "simple_message_server_logic", NULL) == -1)
			{
				fprintf(stderr, "%s: execlp() failed: %s\n", prg_name, strerror(errno));
				return EXIT_FAILURE;
			}
			close(new_socket_desc);
		}
		close(new_socket_desc);
	}

	return EXIT_SUCCESS;

}
/**
 *
 * \brief check_parameters_server function checks parameters and reacts accordingly
 *
 *
 * \param argc passes the number of arguments
 * \param argv passes the arguments (programme name is argv[0]
 * \param port gets the port for the server
 *
 *
 */

void check_parameters_server(int argc, char *argv[], const char **port)
{
	int j;
	long int port_number;
	char *end_ptr;

	struct option long_options[] =
	{
			{"port", 1, NULL, 'p'},
			{"help", 0, NULL, 'h'},
			{0, 0, 0, 0}
	};

	*port = NULL;

	while ((j = getopt_long(argc, (char **const) argv, "p:h", long_options, NULL)) != -1)
	{
		switch(j)
		{
		case 'p':
			port_number = strtol(optarg, &end_ptr, STRTOL_BASE);

			/* if strtol failed */
			if(errno == ERANGE)
			{
				fprintf(stderr, strerror(errno));
			}

			/* check if port is within range */
			if(port_number < PORT_MIN || port_number > PORT_MAX)
			{
				fprintf(stderr, "%s: port out of range", prg_name);
				my_usage(stderr, EXIT_FAILURE);
			}
			/* save port */
			*port = optarg;
			break;
		case 'h':
			my_usage(stdout, EXIT_SUCCESS);
			break;
		case '?':
			my_usage(stderr, EXIT_FAILURE);
			break;
		default:
			my_usage(stderr, EXIT_FAILURE);
			break;
		}
	}

	if((optind != argc) || (port == NULL))
	{
		my_usage(stderr, EXIT_FAILURE);
	}
}

/**
 *
 * \brief Main function implements a server which connects to a client
 * Main entry point
 *
 * \param sig
 *
 *
 *
 */
void signal_child(int sig)
{
	/* to prevent warnings, no other use */
	sig = sig;
	/* wait for any child process and return immediately if no child has exited */
	while(waitpid(-1, NULL, WNOHANG) > 0);
}
/**
 *
 * \brief my_usage function prints usage function
 *
 *
 * \param sig
 *
 *
 */
void my_usage(FILE * out, int exit_status)
{
	/*variable for error handling */
	int check;

	check = fprintf(out, "usage: %s <options>\n"
			"\t-p, \t--port <port>\n"
			"\t-h, \t--help\n", prg_name);
	/* if fprintf to stdout fails and flush after that */
	if(check < 0)
	{
		fprintf(stderr, strerror(errno));
	}
	fflush(out);
	fflush(stderr);

	exit(exit_status);
}


