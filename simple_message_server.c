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
#include <error.h>
#include <stdarg.h>

/*
 * ---------------------------------- globals ------------------------
 */
#define LISTEN 24

const char *prg_name;


/*
 * ---------------------------------- function prototypes ------------
 */

void check_params(int argc, char *argv[], const char **port);
void my_usage(void);
void my_printf(char * format, ...);
void check_parameters_server(int argc, char *argv[], const char **port);


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
	struct sockaddr_storage address;
	socklen_t address_length;
	//ssize_t read;
	/* server port number */
	const char *port = NULL;
	int y = 0;

	prg_name = argv[0];
	check_parameters_server(argc, argv, &port);

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
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
	}

	return EXIT_SUCCESS;

}
/* meine Alternative - darf aber wohl nicht verwendet werden wg. getopt, das verwendet werden muss */
void check_params(int argc, char *argv[], const char **port)
{

	/* if less than three arguments are passed, the usage is not correct */
	if (argc < 2)
	{
		my_printf("Check argc\n");
		my_usage();
	}
	else if ((strcmp(argv[0], "./simple_message_server")) != 0)
	{
		my_printf("Check arg 0");
		my_usage();
	}
	else if(((strcmp(argv[1], "-h")) || (strcmp(argv[1], "--h")) || (strcmp(argv[1], "?"))) == 0)
	{
		my_printf("check for help \n");
		my_usage();
	}
	else if(((strcmp(argv[1], "-p")) || (strcmp(argv[1], "--p"))) == 0)
	{
		my_printf("You choose port: ");
		*port = argv[2];
	}


}
void check_parameters_server(int argc, char *argv[], const char **port)
{
	int j;
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
			*port = optarg;
			break;
		case 'h':
			my_usage();
			break;
		case '?':
			my_usage();
			break;
		default:
			my_usage();
			break;
		}
	}

	if((optind != argc) || (port == NULL))
	{
		my_usage();
	}
}

void my_usage(void)
{
	my_printf("usage: %s <options>\n"
			"\t-p, \t--port <port>\n"
			"\t-h, \t--help\n", prg_name);
	exit(EXIT_FAILURE);
}

void my_printf(char * format, ...)
{
	va_list args;

	va_start(args, format);

	if (vprintf(format, args) < 0)
		error(1, 1, "%d", errno);

	va_end(args);
}
