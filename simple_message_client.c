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
#include <arpa/inet.h>
#include <ctype.h>
#include <stdarg.h>
#include "simple_message_client_commandline_handling.h"

/*
 * ---------------------------------- defines ------------------------
 */

/*
 * ---------------------------------- globals ------------------------
 */

const char *prg_name;
int verbose;

/*
 * ---------------------------------- function prototypes ------------
 */


static void usage(FILE *out, const char *prg_name, int exit_status);
void logger(char *message);
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
	//size_t length;
	int check;
	int socket_desc;
	int connect_socket;
	const char *server = NULL;
	const char *port = NULL;
	const char *user = NULL;
	const char *message = NULL;
	const char *image = NULL;



	smc_parsecommandline(argc, argv, &usage, &server, &port, &user, &message, &image, &verbose);
	prg_name = argv[0];

	memset(&client_info, 0, sizeof(client_info));
	client_info.ai_family = AF_UNSPEC;
	client_info.ai_socktype = SOCK_STREAM;
	client_info.ai_protocol = 0;

	check = getaddrinfo(server, port, &client_info, &set_info);
	if(check != 0)
	{
		fprintf(stderr, "%s: getaddrinfo failed: %s\n", prg_name, gai_strerror(check));
		return EXIT_FAILURE;
	}

	/* go through all he results and connect if possible - if not, try the next one */
	for (rp = set_info; rp != NULL; rp = rp->ai_next)
	{
		socket_desc = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
		if (socket_desc == -1)
		{
			logger("socket");
			continue;
		}
		/* if connect is successful, break and close socket descriptor */
		connect_socket = connect(socket_desc, rp->ai_addr, rp->ai_addrlen);
		if(connect_socket == -1)
		{
			logger("connect");
			close(socket_desc);
			continue;
		}
		else
			break;
	}

	if(rp == NULL)
	{
		fprintf(stderr, "%s: could not connect to any address.\n", prg_name);
		freeaddrinfo(set_info);
		return EXIT_FAILURE;
	}
	/* is no longer needed */
	freeaddrinfo(set_info);






}
/**
 * \brief a function pointer to a function which is called from smc_parsecommandline() if the user enters wrong
 *        parameters.
 *        The type of  this  function pointer is: typedef void (* smc_usagefunc_t) (FILE *, const char *, int);
 *
 * \param out      - specifies if ouput is STDOUT or STDIN
 * \param prg_name  - a constant character array containing the name of the executed programme  (i.e., the contents of argv[0]).
 * \param exit_status - the exit code to be used in the call to exit(exit_status) for terminating the programme.
 */
static void usage(FILE *out, const char *prg_name, int exit_status)
{
	fprintf(out,"usage: %s options\n",prg_name);
	    fprintf(out,"options:\n");
	    fprintf(out,"\t-s, --server <server>   full qualified domain name or IP address of the server\n");
	    fprintf(out,"\t-p, --port <port>       well-known port of the server [0..65535]\n");
	    fprintf(out,"\t-u, --user <name>       name of the posting user\n");
	    fprintf(out,"\t-i, --image <URL>       URL pointing to an image of the posting user\n");
	    fprintf(out,"\t-m, --message <message> message to be added to the bulletin board\n");
	    fprintf(out,"\t-v, --verbose           verbose output (for debugging purpose)\n");
	    fprintf(out,"\t-h, --help\n");

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
void logger(char *message)
{
	int j = 0;
	if (verbose == TRUE)
	{
		j = fprintf(stdout, "%s: %s\n", prg_name, message);

		/* if printig to stdout fails, a negative value is returned */
		if(j < 0)
		{
			fprintf(stderr, "Can not print to stdout.\n");
		}
	}
}
void my_printf(char * format, ...)
{
	va_list args;

	va_start(args, format);

	if (vprintf(format, args) < 0)
		error(1, 1, "%d", errno);

	va_end(args);
}

