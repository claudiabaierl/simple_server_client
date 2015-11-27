/**
 * @file simple_message_client.c
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

#include <stdio.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "simple_message_client_commandline_handling.h"

/*
 * ---------------------------------- defines ------------------------
 */

/*
 * ---------------------------------- globals ------------------------
 */

const char *prg_name;

/*
 * ---------------------------------- function prototypes ------------
 */


static void usage(FILE *out, const char *prg_name, int exit_status);
void my_printf(char *format, ...);



/**
 *
 * \brief Main function is the entry point for any C programme
 *
 * \param argc passes the number of arguments
 * \param argv passes the arguments (programme name is argv[0]
 *
 * \return
 *
 */

int main(int argc, char *argv[])
{

	struct addrinfo client_info;
	struct addrinfo *set_info, *rp;
	size_t length;
	int addr_info;
	int socket_desc;
	const char *server = NULL;
	const char *port = NULL;
	const char *user = NULL;
	const char *message = NULL;
	const char *image = NULL;

	prg_name = argv[0];

	smc_parsecommandline(argc, argv, &usage, &server, &port, &user, &message, &image, &verbose);


	memset(&client_info, 0, sizeof(client_info));
	client_info.ai_family = AF_UNSPEC;
	client_info.ai_socktype = SOCK_STREAM;
	client_info.ai_protocol = 0;

	add_info = getaddrinfo(argv[1], argv[2], &client_info, &set_info);
	if(s != 0)
	{
		fprintf(stderr, "getaddrinfo failed: %s\n", gai_sterror(s));
		exit(EXIT_FAILURE);
	}

	for (rp = set_info; rp != NULL, rp = rp->ai_next)
	{
		sockt_desc = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
		if (sock_desc == -1)
		{
			continue;
		}
		/* if connect is successful, break and close socket descriptor */
		if(connect(socket_desc, rp->ai_addr, rp->ai_addrlen) != -1)
			break;
		close(sock_desc);
	}

	if(rp == NULL)
	{
		fprintf(stderr, "Could not connect to any address. \n");
		exit(EXIT_FAILURE);
	}

	freeaddrinfo(client_info);






}
static void usage(FILE *out, const char *prg_name, int exit_status)
{
	my_printf("Usage: %s options\n"
			"options:"
			"\t -s, --server <server>\t full qualified domain name or IP address of the server\n"
			"\t -p, --port <port>\t well-known port of the server [0..65535]\n"
			"\t -u, --user <name>\t name of the posting user\n"
			"\t -i, --image <URL>\t URL pointing to an image of the posting user\n"
			"\t -m, --message <message>\t messag to be added to the bulletin board\n"
			"\t -v, --verbose\t verbose output\n"
			"\t -h, --help\n", prg_name);

	exit(exit_status);
}
/**
 *
 * \brief Function error handling of printf
 *
 * \param format
 *
 *
 */
void my_printf(char * format, ...)
{
	va_list args;

	va_start(args, format);

	if (vprintf(format, args) < 0)
		error(1, 1, "%d", errno);

	va_end(args);
}

