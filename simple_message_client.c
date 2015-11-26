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

	prg_name = argv[0];



	memset(&client_info, 0, sizeof(struct addrinfo));
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
