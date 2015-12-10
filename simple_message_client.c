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
 #include <assert.h>
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
long int status;

/*
 * ---------------------------------- function prototypes ------------
 */


static void usage(FILE *out, const char *prog_name, int exit_status);
void logger(char *message);
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
		fprintf(stderr, "%s: Cannot connect() to socket - %s\n", prg_name, strerror(errno));
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

		verbose_print(", %s(), line %d] Sent request user =\"%s\"\n",  __func__, __LINE__, user);

		/*user field is required - don't have to check again if username was entered*/
		/*check message data and send*/
		/*only send image tag if image was given*/
		if(image==NULL)
		{
			verbose_print(", %s(), line %d] message =\"%s\n",  __func__, __LINE__, message);
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
			verbose_print(", %s(), line %d] img=\"%s\n",  __func__,__LINE__, image);
			verbose_print(", %s(), line %d] message =\"%s\n", __func__, __LINE__,message);

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
			fprintf(stderr, "%s: failed to close writing direction - %s\n", prg_name, strerror(errno));
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
	
	char receive_buffer[MAXIMUM_SIZE];
	char value[MAXIMUM_SIZE];
	char *end_ptr;
	int file_length_received = 0;
	int maximum_buffer =0;
	int count = 0;
	int bytes_received = 0;
	int bytes_read = 0;

	client_socket = fdopen(socket_desc, "r");
	if(client_socket == NULL)
	{
		fprintf(stderr, "%s: failed to open socket for reading - %s\n", prg_name,  strerror(errno));
		logger("open file descriptor");
		return EXIT_FAILURE;
	}

	while(fgets(receive_buffer, 2048, client_socket) != NULL)
	{
		switch(mode)
		{
		case 0:
			if(check_stream(buffer, "status=", value) == 0)
			{
				status = strtol(value, &end_ptr, 10);
				if(value == end_ptr)
				{
					fprintf(stderr, "Failed converting status to integer - %s\n",  strerror(errno));
					verbose_print(", %s(), line %d] Failed to convert status\n",  __func__,__LINE__);
					my_close(client_socket);
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
					fprintf(stderr, "Unable to open file - %s\n",  strerror(errno));
					verbose_print(", %s(), line %d] Unable to open file: %s\n",  __func__, __LINE__, value);
					my_close(client_socket);
					return EXIT_FAILURE;
				}
				mode = 2;
			}
		case 2:
			if(check_stream(buffer, "len=", value) == 0)
			{
				file_length_received = strtol(value, &end_ptr, 10);
				if(value == end_ptr)
				{
					fprintf(stderr, "Error converting file length to integer - %s\n",  strerror(errno));
					verbose_print(", %s(), line %d] File length could not be converted to integer\n",  __func__, __LINE__);
					my_close(client_socket);
					my_close(write_to);
					return EXIT_FAILURE;
				}
				mode = 3;
			}
		case 3:
			if(write_to == NULL)
			{
				fprintf(stderr, "Something went wrong - now file was opened - %s\n",  strerror(errno));
				verbose_print(", %s(), line %d] No file was opened\n",  __func__, __LINE__);
				my_close(client_socket);
				return EXIT_FAILURE;
			}
		default: assert(0);
		}
	}

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
		verbose_print(", %s(), line %d] Read: %d @%d byte\n",  __func__, __LINE__, count, maximum_buffer);

		/* read data from the socket */
		bytes_read = fread(buffer, sizeof(char), maximum_buffer, client_socket);
		bytes_received = bytes_received + maximum_buffer;

		/* if not as many bytes were read as written, an error occured */
		if ((int) fwrite(buffer, sizeof(char), bytes_read, write_to) != bytes_read)
		{
			my_close(write_to);
			my_close(client_socket);
			fprintf(stderr, "Error while writing to file");
			return EXIT_FAILURE;
		}

		/* if the received bytes are the same as the file length, check if there is another record
		 * and close everything here
		 */
		if(bytes_received == file_length_received)
		{
			my_close(write_to);
			my_close(client_socket);
			logger("Done with file reading - look for the next record");
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
	logger("Received EOF");
	my_close(client_socket);

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
		new_position = NULL; 
		
		if(position == NULL)
		{
			verbose_print(", %s(), line %d] Value not found in stream: %s\n",  __func__, __LINE__, stream);
		}
		else
		{
			position += strlen(lookup);
			new_position = strchr(position, '\n');
		}

		if(new_position == NULL)
		{
			verbose_print(", %s(), line %d] New line character not found, %s\n",  __func__, __LINE__, position);
		}

		/* copy found value in given variable to pass */
		memset(value, 0, strlen(value));
		strncpy(value, position, (new_position - position));

		/* position the pointer after the value, so we can search the next item */
		stream = stream + strlen(lookup) + strlen(value) + 1;

		verbose_print(", %s(), line %d] check_stream(): check for %, Value %s\n",  __func__, __LINE__,  lookup, value);
		return 0;
	}

	return -1;
}

/**
 * \brief a function pointer to a function which is called from smc_parsecommandline() if the user enters wrong
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
			fprintf(stderr, "Can not print to stdout - %s\n",  strerror(errno));
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


