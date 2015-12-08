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
#include <stdarg.h>
#include "simple_message_client_commandline_handling.h"

/*
 * ---------------------------------- defines ------------------------
 */

#define QUEUESIZE 10 //number of pending connections for connection queue
#define MAXIMUM_SIZE 2048

/*
 * ---------------------------------- globals ------------------------
 */

const char *prg_name;
static int verbose = 0;
static long int maximum;

/*
 * ---------------------------------- function prototypes ------------
 */


static void usage(FILE *out, const char *prog_name, int exit_status);
void logger(char *message);
void *get_in_addr(struct sockaddr *sa);
int send_message(int socket_desc, const char *user, const char *message, const char *image);
int receive_response(int socket_desc);
int get_max(void);
void verbose_print(const char *format, ...);
int check_stream(char *stream, const char *lookup, char *value);


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


		/*open file and associate it to the stream socket*/
		message_desc = fdopen(socket_desc, "w");
		if(message_desc == NULL)
		{
			fclose(message_desc);
			logger("file open");
			return EXIT_FAILURE;
		}

		verbose_print("Sent request user =\"%s\"", user);

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
			verbose_print("user=\"%s\"img=\"%s", image);


			send_message = fprintf(message_desc,"user=%s\nimg=%s\n%s\n",user, image, message);
				if (send_message ==-1)
				{
					fclose(message_desc);
					logger("message");
					return EXIT_FAILURE;
				}
		}

		/*write all unwritten data to the file/socket */
		flush_check = fflush(message_desc);
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
	char buffer[MAXIMUM_SIZE];
	int mode = 0;
	long int status;
	char receive_buffer[MAXIMUM_SIZE];
	char value[MAXIMUM_SIZE];
	char *end_ptr;
	int file_length, maximum_buffer;
	int rcv = 0;
	int count = 0;

	client_socket = fdopen(socket_desc, "r");
	if(client_socket == NULL)
	{
		fprintf(stderr, "%s: failed to open socket for reading", prg_name);
		logger("open file descriptor");
		return EXIT_FAILURE;
	}

	while(fgets(receive_buffer, 2048, socket_desc) != NULL)
	{
		switch(mode)
		{
		case 0:
			if(check_stream(buffer, "status=", value) == 0)
			{
				status = strtol(value, &end_ptr, 10);
				if(value == end_ptr)
				{
					fprintf(stderr, "Failed converting status to integer");
					verbose_print("Failed to convert status");
					close(socket_desc);
					return EXIT_FAILURE;
				}

			}
			mode = 1;
			break;
		case 1:
			if(check_stream(buffer, "file=", value) == 0)
			{
				write_to = fopen(value, "w");
				if(write_to == NULL)
				{
					fprintf(stderr, "Unable to open file");
					verbose_print("Unable to open file: %s", value);
					close(socket_desc);
					return EXIT_FAILURE;
				}
				mode = 2;
			}
		case 2:
			if(check_stream(buffer, "len=", value) == 0)
			{
				file_length = strtol(value, &end_ptr, 10);
				if(value == end_ptr)
				{
					fprintf(stderr, "Error converting file length to integer");
					verbose_print("File length could not be converted to integer");
					close(socket_desc);
					close(write_to);
					return EXIT_FAILURE;
				}
				mode = 3;
			}
		case 3:
			if(write_to == NULL)
			{
				fprintf(stderr, "Something went wrong - now file was opened");
				verbose_print("No file was opened");
				close(socket_desc);
				return EXIT_FAILURE;
			}
		default:
			assert(0);
		}
	}

	while(1)
	{
		count++;
		if(MAXIMUM_SIZE < (file_length-received))
		{
			maximum_buffer = MAXIMUM_SIZE;
		}
		else
		{
			maximum_buffer = file_length-received;
		}
		verbose_print("Read: %d @%d byte", count, maximum_buffer);
	}





	return EXIT_SUCCESS;

}
/**
 * \brief check_stream Function checks for values and returns that if found
 *
 * \param stream  data from tcp stream
 * \param lookup   value that is looked for
 * \param value  found value that is returned (return value!)
 *
 * \return      0 when successful and -1 if not successful
 */
int check_stream(char *stream, const char *lookup, char *value)
{
	char *position, *new_position;

	if(stream != NULL)
	{
		position = strstr(lookup, value);

		if(position == NULL)
		{
			verbose_print("Value not found in stream: %s", stream);
		}
		else
		{
			position += strlen(lookup);
			new_position = strchr(position, "\n");
		}

		if(new_position == NULL)
		{
			verbose_print("New line character not found, %s", key);
		}

		/* copy found value in given variable to pass */
		memset(value, 0, strlen(value));
		strncpy(value, position, new_position - position);

		/* position the pointer after the value, so we can search the next item */
		stream = stream + strlen(lookup) + strlen(value) + 1;

		verbose_print("check_stream(): check for %, Value %s", lookup, value);
		return 0;
	}

	return -1;
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
static void usage(FILE *out, const char *prog_name, int exit_status)
{
	int check;

	check = fprintf(out,"usage: %s options\n",prog_name);
	    fprintf(out,"options:\n");
	    fprintf(out,"\t-s, --server <server>   full qualified domain name or IP address of the server\n");
	    fprintf(out,"\t-p, --port <port>       well-known port of the server [0..65535]\n");
	    fprintf(out,"\t-u, --user <name>       name of the posting user\n");
	    fprintf(out,"\t-i, --image <URL>       URL pointing to an image of the posting user\n");
	    fprintf(out,"\t-m, --message <message> message to be added to the bulletin board\n");
	    fprintf(out,"\t-v, --verbose           verbose output (for debugging purpose)\n");
	    fprintf(out,"\t-h, --help\n");


	    if (check < 0)
	    {
	    	fprintf(stderr, "fprintf failed: %s", strerror(errno));
	    }

	    fflush(out);
	    fflush(stderr);

	exit(exit_status);
}
/**
 *
 * \brief logger Function error handling of printf
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
/**
 *
 * \brief get_max Function getting max size
 *
 * \return EXIT_SUCCESS if no error occurs
 * \return EXIT_FAILURE if an error occurs
 *
 */
int get_max(void)
{

	maximum = pathconf(".", _PC_NAME_MAX);
	if(maximum == -1)
	{
		fprintf(stderr, "%s: getting max path failed", prg_name);
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;

}
/**
 *
 * \brief verbose_print Function prints to stdout if -v is activated
 *
 * \return EXIT_SUCCESS if no error occurs
 * \return EXIT_FAILURE if an error occurs
 *
 */
void verbose_print(const char *format, ...)
{
	va_list argp;
	int n;

	if(verbose != 0)
	{
		fprintf(stdout, "Verbose: ");
		va_start(argp, format);
		n = vfprintf(stdout, format, argp);

		if(n < 0)
		{
			fprintf(stderr, strerror(errno));
			exit(EXIT_FAILURE);
		}

		va_end(argp);
	}
}


