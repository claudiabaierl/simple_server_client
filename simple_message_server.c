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
 * ---------------------------------- globals ------------------------
 */

const char *prg_name;


/*
 * ---------------------------------- function prototypes ------------
 */

void check_params(int argc, char *argv[]);
void usage(FILE *stream, const char *command, int exit_status);
void my_usage(void);
void my_printf(char * format, ...);


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


	prg_name = argv[0];
	check_params(argc, argv);



}

void check_params(int argc, char *argv[])
{

	/* if less than three arguments are passed, the usage is not correct */
	if (argc < 3)
	{
		my_usage();
	}
	else if (strcmp(argv[0], "simple_message_server") != 0)
	{
		my_usage();
	}
	else if(strcmp(argv[1], "-p") != 0)
	{
		my_usage();
	}
	else if(strcmp(argv[1], "-h") != 0)
	{
		my_usage();
	}


}

void usage(FILE *stream, const char *command, int exit_status)
{
	fprintf(stream, "usage: %s <options>\n", command);
	fprintf(stream, "options:\n");
	fprintf(stream, "\t-p, \t--port <port>");
	fprintf(stream, "\t-h, \t--help\n");
	exit(exit_status);

}
void my_usage(void)
{
	myprintf("usage: %s <options>\n"
			"\t-p, \t--port <port>"
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
