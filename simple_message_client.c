/**
 * @file simple_message_client.c
 *
 * VCS TCP/IP Client
 *
 * @author: Claudia Baierl - ic14b003 <ic14b003@technikum-wien.at>
 * @author: Zuebide Sayici - ic14b002 <ic14b002@technikum-wien.at>
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
#include "simple_message_client_commandline_handling.h"

/*
 * ---------------------------------- defines ------------------------
 */

#define QUEUESIZE 10 //number of pending connections for connection queue

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
void *get_in_addr(struct sockaddr *sa);
int send_message(int socket_desc, const char *user, const char *message, const char *image);
int receive_response(int socket_desc);


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

int main(int argc, const char * const argv[])
{

	struct addrinfo client_info, *set_info, *rp;

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

	//client_info is set to 0
	memset(&client_info, 0, sizeof(client_info));
	client_info.ai_family = AF_UNSPEC; /*not specified if IPv4 or IPv6*/
	client_info.ai_socktype = SOCK_STREAM; /*socket type = stream*/
	client_info.ai_protocol = 0;


	check = getaddrinfo(server, port, &client_info, &set_info); /*get addrinfo structs - contains internet address etc.*/
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


		connect_socket = connect(socket_desc, rp->ai_addr, rp->ai_addrlen);
		if(connect_socket == -1)
		{
			logger("connect");
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

	send_message(socket_desc, user, message, image);

	/*close the socket connection*/
	close(socket_desc);
	return 0;

}


/**
 *
 * \brief send_message function sends user, message and if chosen by user an image
 *
 * \param socket_desc passes the socket_descriptor
 * \param user passes the user who sends the message
 * \param message passes the message to written to the bulletin board
 * \param image passes the image URL to be shown in the bulletin board
 *
 * \return EXIT_SUCCESS when no error occurs
 * \return EXIT_FAILURE on error
 *
 */
int send_message(int socket_desc, const char *user, const char *message, const char *image)
{

	int send_message;
	int flush_check;
	FILE *message_desc = NULL;
	char verbose[256];

	/*wofür brauchen wir das??*/
	//fprintf(stderr, "image:%s", image);

		/*open file and associate it to the stream socket*/
		message_desc = fdopen(socket_desc, "w");
		if(message_desc == NULL)
		{
			fclose(message_desc);
			logger("file open");
			return EXIT_FAILURE;
		}

		if(sprintf(verbose, "Sent request user=\"%s\"", user) < 0)
		{
			fprintf(stderr, "%s: failed to write verbose string", prg_name);
		}

		/*user field is required - don't have to check again if username was entered*/
		/*check message data and send*/
		/*only send image tag if image was given*/
		if(image==NULL)
		{
			send_message = fprintf(message_desc,"user=%s\n%s\n", user, message);
				if (send_message ==-1)
				{
					fclose(message_desc);
					logger("message");
					return EXIT_FAILURE;
				}
		}
		/*message is required - send image and message if image is given*/
		else
		{
			if(sprintf(verbose, "%s, img=\"%s\"", verbose, image) < 0)
			{
				fprintf(stderr, "%s: failed to write verbose string", prg_name);
			}

			send_message = fprintf(message_desc,"user=%s\nimg=%s\n%s\n",user, image, message);
				if (send_message ==-1)
				{
					fclose(message_desc);
					logger("message");
					return EXIT_FAILURE;
				}
		}

		/*write all unwritten data to the file/socket */
		flush_check =fflush(message_desc);
		if (flush_check != 0)
		{

			fclose(message_desc);
			logger("flush");
			return EXIT_FAILURE;
		}

		if(shutdown(socket_desc, SHUT_WR) != 0)
		{
			fprintf(stderr, "%s: failed to close writing direction", prg_name);
			logger("shutdown");
			return EXIT_FAILURE;
		}

	logger(verbose);

	return EXIT_SUCCESS;

}
/**
 *
 * \brief send_message function receives the servers response
 *
 * \param socket_des passes the socket descriptor
 *
 * \return EXIT_SUCCESS when no error occurs
 * \return EXIT_FAILURE on error
 *
 */
int receive_response(int socket_desc)
{
	FILE *client_socket;
	FILE *write_to = NULL;
	char buffer[256];



	client_socket = fdopen(socket_desc, "r");
	if(client_socket == NULL)
	{
		fprintf(stderr, "%s: failed to open socket for reading", prg_name);
		logger("open file descriptor");
		return EXIT_FAILURE;
	}




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



