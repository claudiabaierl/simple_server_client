/**
 * @file simple_message_client.c
 *
 * VCS TCP/IP Client
 *
 * @author: Claudia Baierl - ic14b003 <ic14b003@technikum-wien.at>
 * @author: Zuebide Sayici - ic14b002 <ic14b002@technikum-wien.at>
 *
 * @version $Revision: 512 $
 *
 * Last Modified: $Author: Zuebide Sayici $
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
#include <assert.h>
#include "simple_message_client_commandline_handling.h"

/*
 * ---------------------------------- defines ------------------------
 */

#define QUEUESIZE 10 /*number of pending connections for connection queue */
#define MAXIMUM_SIZE 2048

/*
 * ---------------------------------- globals ------------------------
 */

const char *prg_name;
static int verbose = 0;
static long int maximum;
int status;

/*
 * ---------------------------------- function prototypes ------------
 */


static void usage(FILE *out, const char *prog_name, int exit_status);
int send_message(int socket_desc, const char *user, const char *message, const char *image);
int receive_response(int socket_desc);
int get_max(void);
void verbose_print(const char *format, ...);
int check_stream(char *stream, const char *lookup, char *value);
void my_close(FILE *fp);


/**
 *
 * \brief Main function is the entry point for any C programme
 *
 * \param argc passes the number of arguments
 * \param argv passes the arguments (programme name is argv[0])
 *
 * \return EXIT_FAILURE if an error occurs
 * \return 0 if no error occurs
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
	/*not specified if IPv4 or IPv6 - both can be used*/
	client_info.ai_family = AF_UNSPEC; 
	/*socket type = tcp*/
	client_info.ai_socktype = SOCK_STREAM; 
	/* protocolype is automatically set (0) */
	client_info.ai_protocol = 0;

	/*get addrinfo structs for the given server and port - contains internet address etc.*/
	check = getaddrinfo(server, port, &client_info, &set_info); 
	if(check != 0)
	{
		fprintf(stderr, "%s: getaddrinfo failed: %s\n", prg_name, gai_strerror(check));
		return EXIT_FAILURE;
	}

	/* go through all he results and connect if possible - if not, try the next one */
	for (rp = set_info; rp != NULL; rp = rp->ai_next)
	{
		/* get the socket file descriptor */
		socket_desc = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
		if (socket_desc == -1)
		{
			continue;
		}

		/* connect to the socket  with connect*/
		connect_socket = connect(socket_desc, rp->ai_addr, rp->ai_addrlen);
		if(connect_socket == -1)
		{
			continue;
		}


		else
			break;
	}

	if(rp == NULL)
	{
		fprintf(stderr, "%s: Cannot connect() to socket - %s\n", prg_name, strerror(errno));
		freeaddrinfo(set_info);
		return EXIT_FAILURE;
	}

	/* is no longer needed */
	freeaddrinfo(set_info);

	send_message(socket_desc, user, message, image);
	receive_response(socket_desc);

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


		/*open file and associate it to the stream socket -writing mode - stream is positioned at beginning of the file*/
		message_desc = fdopen(socket_desc, "w");
		if(message_desc == NULL)
		{
			fprintf(stderr, "%s: failed open file for message %s\n", prg_name, strerror(errno));
			my_close(message_desc);
			return EXIT_FAILURE;
		}

		verbose_print(", %s(), line %d] Sent request user =\"%s\"\n",  __func__, __LINE__, user);

		/*user field is required - don't have to check again if username was entered*/
		/*check message data and send*/
		/*only send image tag if image was given*/
		if(image == NULL)
		{
			verbose_print(", %s(), line %d] message =\"%s\n",  __func__, __LINE__, message);
			/* send the message to the stream */
			send_message = fprintf(message_desc,"user=%s\n%s\n", user, message);
				if (send_message == -1)
				{
					fprintf(stderr, "%s: failed to send message - %s\n", prg_name, strerror(errno));
					my_close(message_desc);
					return EXIT_FAILURE;
				}
		}
		/*message is required - send image and message if image is given*/
		else
		{
			verbose_print(", %s(), line %d] img=\"%s\n",  __func__,__LINE__, image);
			verbose_print(", %s(), line %d] message =\"%s\n", __func__, __LINE__,message);
			/* send the message to the stream */
			send_message = fprintf(message_desc,"user=%s\nimg=%s\n%s\n",user, image, message);
				if (send_message ==-1)
				{
					fprintf(stderr, "%s: failed to send message - %s\n", prg_name, strerror(errno));
					my_close(message_desc);
					return EXIT_FAILURE;
				}
		}

		/*write all unwritten data to the file/socket */
		flush_check = fflush(message_desc);
		if (flush_check != 0)
		{
			fprintf(stderr, "%s: failed to flush socket - %s\n", prg_name, strerror(errno));
			my_close(message_desc);
			return EXIT_FAILURE;
		}

		/*block further sending*/
		if(shutdown(socket_desc, SHUT_WR) != 0)
		{
			fprintf(stderr, "%s: failed to close writing direction - %s\n", prg_name, strerror(errno));
			return EXIT_FAILURE;
		}
		
		/*close the socket descriptor*/
		my_close(message_desc);

	return EXIT_SUCCESS;

}
/**
 *
 * \brief receive_response function receives the servers response
 *
 * \param socket_desc passes the socket descriptor
 *
 * \return EXIT_SUCCESS when no error occurs
 * \return EXIT_FAILURE on error
 *
 */
int receive_response(int socket_desc)
{
	FILE *client_socket;
	FILE *write_to = NULL;

	int mode;
	
	char receive_buffer[MAXIMUM_SIZE];
	char value[MAXIMUM_SIZE];
	char *end_ptr;
	int file_length_received = 0;
	int maximum_buffer = 0;
	int count = 0;
	int bytes_received = 0;
	int bytes_read = 0;

	/*open the file for reading*/
	client_socket = fdopen(socket_desc, "r");
	if(client_socket == NULL)
	{
		fprintf(stderr, "%s: failed to open socket for reading - %s\n", prg_name,  strerror(errno));
		return EXIT_FAILURE;
	}
	verbose_print(", %s(), line %d] Client_Socket is open.\n",  __func__, __LINE__);
	mode = 0;
	
	/*read characters from stream and store them into receive_buffer*/
	while(fgets(receive_buffer, MAXIMUM_SIZE, client_socket) != NULL)
	{
		switch(mode)
		{
		case 0:
		
			/*check if "status=" was received*/
			if(check_stream(receive_buffer, "status=", value) == 0)
			{
				/*store received status into status variable*/
				if(sscanf(receive_buffer,"status=%d",&status) == 0)
				{
					fprintf(stderr, "Failed to retrieve status. %s", strerror(errno));
					my_close(client_socket);
					return EXIT_FAILURE;
				}
				/*if status = 0 go ahead*/
				if(status == 0)
				{
					mode = 1;
				}
				else
				{
					verbose_print(", %s(), line %d] Status: %d is invalid\n",  __func__, __LINE__, status);
					fprintf(stderr, "Wrong status");
				}
			}
			break;
		case 1:
			/*check if file was received*/
			if(check_stream(receive_buffer, "file=", value) == 0)
			{
				/*try to open a new file for writing*/
				write_to = fopen(value, "w");
				if(write_to == NULL)
				{
					fprintf(stderr, "Unable to open file - %s\n",  strerror(errno));
					verbose_print(", %s(), line %d] Unable to open file: %s\n",  __func__, __LINE__, value);
					my_close(client_socket);
					return EXIT_FAILURE;
				}
			}
			/*switch to next check*/
			mode = 2;
			break;
		case 2:
			/*check if "len=" was received*/
			if(check_stream(receive_buffer, "len=", value) == 0)
			{
				/*convert string to long integer*/
				file_length_received = strtol(value, &end_ptr, 10);
				if(value == end_ptr)
				{
					fprintf(stderr, "Error converting file length to integer - %s\n",  strerror(errno));
					verbose_print(", %s(), line %d] File length could not be converted to integer\n",  __func__, __LINE__);
					my_close(client_socket);
					my_close(write_to);
					return EXIT_FAILURE;
				}
			}
			/*switch to next check*/
			mode = 3;
			break;
		default:
			if(write_to == NULL)
			{
				fprintf(stderr, "Something went wrong - no file was opened - %s\n",  strerror(errno));
				verbose_print(", %s(), line %d] No file was opened\n",  __func__, __LINE__);
				my_close(client_socket);
				return EXIT_FAILURE;
			}
		}

		if(mode == 3)
		{
			bytes_read = 0;
			bytes_received=0;

			while(1)
			{
				count++;
				if(MAXIMUM_SIZE < (file_length_received))
				{
					maximum_buffer = MAXIMUM_SIZE;
				}
				else
				{
					maximum_buffer = file_length_received;
				}
				verbose_print(", %s(), line %d] Read part: %d byte %d\n",  __func__, __LINE__, count, maximum_buffer);

				/* read data from the socket */
				bytes_read = fread(receive_buffer, sizeof(char), maximum_buffer, client_socket);
				verbose_print(", %s(), line %d] Bytes_read called",  __func__, __LINE__);
				bytes_received = bytes_received + maximum_buffer;
				verbose_print(", %s(), line %d] Bytes_received called",  __func__, __LINE__);

				/* if not as many bytes were read as written, an error occurs */
				if ((int) fwrite(receive_buffer, sizeof(char), bytes_read, write_to) != bytes_read)
				{
					my_close(write_to);
					my_close(client_socket);
					fprintf(stderr, "Error while writing to file");
					return EXIT_FAILURE;
				}

				/* if the received bytes are the same as the file length, check if there is 
				* another record and close everything here */
				if(bytes_received >= file_length_received)
				{
					my_close(write_to);
					/* Next record */
					mode = 1;
					break;
				}

				/* if there was a reading error, break */
				if(ferror(client_socket) != 0)
				{
					my_close(write_to);
					my_close(client_socket);
					fprintf(stderr, "Error reading from stream");
					return EXIT_FAILURE;
				}
			}
		}
	}
	verbose_print(", %s(), line %d] EOF reached \n",  __func__, __LINE__);
	my_close(client_socket);
	my_close(write_to);

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
	char *position;

	verbose_print(", %s(), line %d] Bin in Check stream\n",  __func__, __LINE__);

	if(stream != NULL)
	{
		/*current position is pointer of first occurence of lookup within stream*/
		position = strstr(stream, lookup);

		if(position != NULL)
		{
			position = position + strlen(lookup);
			verbose_print(", %s(), line %d] Position ist nicht NULL\n",  __func__, __LINE__);

		verbose_print(", %s(), line %d] Stream is not null.", __func__, __LINE__);
		/* copy found value in given variable to pass */
		memset(value, 0, MAXIMUM_SIZE);
		strncpy(value, position, (strlen(position) - 1));

		verbose_print(", %s(), line %d]: check for %s, Value %s\n",  __func__, __LINE__,  lookup, value);
		return 0;
		}
	}

	return -1;
}

/**
 * \brief usage function pointer to a function which is called from smc_parsecommandline() if the user enters wrong
 *        parameters.
 *        The type of  this  function pointer is: typedef void (* smc_usagefunc_t) (FILE *, const char *, int);
 *
 * \param out      - specifies if ouput is STDOUT or STDERR
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
	    	fprintf(stderr, "fprintf failed: %s\n", strerror(errno));
	    }

	    fflush(out);
	    fflush(stderr);

	exit(exit_status);
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
		fprintf(stderr, "%s: getting max path failed - %s\n", prg_name,  strerror(errno));
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
		fprintf(stdout, "%s [%s",prg_name, __FILE__);
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
/**
 *
 * \brief my_close Function flushes the stream pointed to by the file pointer
 *
 * \param fp points to the stream to be flushed
 *
 *
 */
void my_close(FILE *fp)
{
	int check;

	check = fclose(fp);

	if(check != 0)
	{
		fprintf(stderr, strerror(errno));
		verbose_print(", %s(), line %d] Error closing file: %s\n",  __func__, __LINE__, fp);
		exit(EXIT_FAILURE);
	}
}


